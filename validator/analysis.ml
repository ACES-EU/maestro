open Core.Std
open Ir

let no_log = true

let log str =
  if no_log then ()
  (* else print_string str *)
  else Out_channel.with_file ~append:true "analysis.log" ~f:(fun f ->
      Out_channel.output_string f str)

let start_log () =
  if no_log then ()
  else Out_channel.with_file "analysis.log" ~f:(fun f -> ())

let lprintf fmt = ksprintf log fmt

let lprint_list lst =
  List.iter lst ~f:(fun vn -> lprintf "%s\n" vn)

let canonicalize_in_place statements =
  let rec canonicalize1 sttmt =
    match sttmt with
    | {v=Bop (Eq,{v=Bool false;_},rhs);t} ->
      {v=Not rhs;t}
    | {v=Bop (Eq,lhs,{v=Bool false;_});t} ->
      {v=Not lhs;t}
    | {v=Bop (Eq,{v=Bool true;_},rhs);t} ->
      canonicalize1 rhs
    | {v=Bop (Eq,lhs,{v=Bool true;_});t} ->
      canonicalize1 lhs
    | {v=Bop (Ge,lhs,rhs);t} -> {v=Bop(Le,rhs,lhs);t}
    | {v=Bop (Gt,lhs,rhs);t} -> {v=Bop(Lt,rhs,lhs);t}
    | _ -> sttmt
  in
  List.map statements ~f:canonicalize1

let expand_conjunctions sttmts =
  let rec expand_sttmt = function
    | {v=Bop (And,lhs,rhs);_} -> (expand_sttmt lhs)@(expand_sttmt rhs)
    | x -> [x]
  in
  List.join (List.map sttmts ~f:expand_sttmt)

let rec expand_struct_eq sttmt : tterm list =
  match sttmt.v with
  | Bop(Eq,{v=Struct(_,fds);t=_},rhs) ->
    List.join
      (List.map fds ~f:(fun fd ->
           expand_struct_eq
             ({v=Bop(Eq,fd.value,{v=Str_idx(rhs,fd.name);t=fd.value.t});t=sttmt.t})))
  | Bop(Eq,lhs,{v=Struct(name,fds);t=rt}) ->
    expand_struct_eq {v=Bop(Eq,{v=Struct(name,fds);t=rt},lhs);t=sttmt.t}
  | _ -> [sttmt]

let expand_struct_eqs sttmts =
  List.join (List.map sttmts ~f:expand_struct_eq)

let expand_fixpoints_in_tterm tterm =
  let rec impl tterm =
    match tterm with
    | Apply (fname,args) ->
      begin
        match String.Map.find Function_spec.fixpoints fname with
        | Some body ->
          Some (List.foldi args ~init:body
                  ~f:(fun i acc arg ->
                      replace_term_in_tterm (Id (sprintf "Arg%d" i))
                        arg.v acc)).v
        | None -> None
      end
    | _ -> None
  in
  call_recursively_on_tterm impl tterm

let remove_double_negation sttmt =
  sttmt |> call_recursively_on_tterm (fun term ->
      match term with
      | Not {v=Not tt;t=_} -> Some tt.v
      | _ -> None)

let remove_trues sttmts =
  List.filter sttmts ~f:(function {v=Bool true;t=_} -> false | _ -> true)

let expand_fixpoints sttmts =
  List.map sttmts ~f:expand_fixpoints_in_tterm

let reduce_struct_idxes sttmt =
  sttmt |> call_recursively_on_tterm (function
      | Str_idx ({v=Struct(_,fields);t=_}, field) ->
        Some (List.find_exn fields ~f:(fun {name;value} ->
          name = field)).value.v
      | _ -> None
    )

(* Transform statement list to a canonical form,
   breaking down conjunctions, inlining function definitions,
   and breaking structures into separate fields. *)
let canonicalize sttmts =
  sttmts |>
  remove_trues |>
  canonicalize_in_place |>
  expand_fixpoints |> (*May introduce conjunctions and negations.*)
  List.map ~f:remove_double_negation |>
  expand_conjunctions |>
  expand_struct_eqs |>
  List.map ~f:reduce_struct_idxes |>
  List.dedup

let extract_equalities ass_list =
  List.partition_tf ass_list ~f:(function {v=Bop(Eq,_,_);t=_} -> true | _ -> false)

let remove_trivial_eqs eqs =
  List.filter eqs ~f:(function
        {v=Bop(Eq,lhs,rhs);t=_} -> not (term_eq lhs.v rhs.v)
      | _ -> true)

let tterm_weight tterm = String.length (render_tterm tterm)

let get_replacement_pair a b keep =
  match (List.exists keep ~f:(tterm_contains_term a)),
        (List.exists keep ~f:(tterm_contains_term b)) with
  | true, true -> None
  | true, false ->
    if (tterm_weight a) > (tterm_weight b) + 50 then
      None
    else Some (b,a)
  | false, true ->
    if (tterm_weight b) > (tterm_weight a) + 50 then
      None
    else Some (a,b)
  | false, false ->
    if (tterm_weight a) > (tterm_weight b) then
      Some (a,b)
    else
      Some (b,a)

let reduce_by_eqs (eqs: tterm list) keep (sttmts: tterm list) =
  List.fold eqs ~init:sttmts
    ~f:(fun acc eq ->
         match eq.v with
         | Bop(Eq,lhs,rhs) ->
           begin
             match get_replacement_pair lhs rhs keep with
             | Some (replace_from, replace_to) ->
               List.map acc ~f:(fun sttmt ->
                   if sttmt.v = eq.v then sttmt else
                     replace_term_in_tterm replace_from.v replace_to.v sttmt)
             | None -> eq::acc
           end
         | _ -> failwith "eqs must contain only equalities")

(* The equalities that are not a simple variable renaming:
   anything except (a = b).*)
let get_meaningful_equalities eqs =
  List.filter eqs ~f:(function
      | {v=Bop (Eq,lhs,rhs);t=_} ->
        begin
          match lhs.v,rhs.v with
          | Id _, Id _ -> false
          | _, _ -> true
        end
      | _ -> failwith "only equalities here")

let reduce_by_equalities vars sttmts =
  List.fold_right sttmts ~f:(fun sttmt (sttmts,eqs) ->
      let sttmt = List.fold eqs ~init:sttmt ~f:(fun acc (rfrom,rto) ->
        replace_term_in_tterm rfrom rto acc) in
      match sttmt with
      | {v=Bop (Eq,lhs,rhs);t=_} ->
        begin
          match get_replacement_pair lhs rhs vars with
          | Some (replace_from, replace_to) ->
            (sttmt::(replace_term_in_tterms replace_from.v replace_to.v sttmts),
            (replace_from.v,replace_to.v)::eqs)
          | None -> (sttmt::sttmts,eqs)
        end
      | _ -> (sttmt::sttmts,eqs)) ~init:([],[])
  |> fst
  |> remove_trivial_eqs
  (* let (eqs,other) = extract_equalities sttmts in *)
  (* let eqs = remove_trivial_eqs eqs in *)
  (* (eqs@other) |> *)
  (* reduce_by_eqs eqs vars |> *)
  (* remove_trivial_eqs *)

let simplify vars sttmts =
  lprintf "\n\nover vars: \n";
  lprint_list (List.map vars ~f:render_term);
  let one = canonicalize sttmts in
  lprintf "\ncanonicalized: \n";
  lprint_list (List.map one ~f:render_tterm);
  let two = reduce_by_equalities vars one in
  lprintf "\nreduced:\n";
  lprint_list (List.map two ~f:render_tterm);
  let three = canonicalize two in
  lprintf "\ncanonicalized2: \n";
  lprint_list (List.map three ~f:render_tterm);
  let four = reduce_by_equalities vars three in
  lprintf "\nreduced2: \n";
  lprint_list (List.map four ~f:render_tterm);
  let five = reduce_by_equalities vars four in
  lprintf "\nreduced3: \n";
  lprint_list (List.map five ~f:render_tterm);
  let six = reduce_by_equalities vars five in
  lprintf "\nreduced4: \n";
  lprint_list (List.map six ~f:render_tterm);
  six
  (* let seven = canonicalize six in *)
  (* lprintf "\ncanonicalized4:\n"; *)
  (* lprint_list (List.map seven ~f:render_tterm); *)
  (* seven *)

let get_ids_from_tterm tterm =
  let rec impl = function
    | Bop (_,lhs,rhs) ->
      (impl lhs.v)@(impl rhs.v)
    | Apply (_,args) ->
      List.join (List.map args ~f:(fun arg -> impl arg.v))
    | Id x -> [x]
    | Struct (_,fds) ->
      List.join (List.map fds ~f:(fun fd -> impl fd.value.v))
    | Int _ -> []
    | Bool _ -> []
    | Not tterm -> impl tterm.v
    | Str_idx (tterm,_) -> impl tterm.v
    | Deref tterm -> impl tterm.v
    | Fptr _ -> []
    | Addr tterm -> impl tterm.v
    | Cast (_,tterm) -> impl tterm.v
    | Undef -> []
  in
  List.dedup (impl tterm.v)

let index_assumptions (ass_list:tterm list) =
  List.fold ass_list ~init:String.Map.empty ~f:(fun acc ass ->
      let ids = get_ids_from_tterm ass in
      let ass_with_ids = (ass,ids) in
      List.fold ids ~init:acc ~f:(fun acc id ->
          String.Map.add acc ~key:id
            ~data:(match String.Map.find acc id with
                | Some ass_list -> ass_with_ids::ass_list
                | None -> [ass_with_ids])))

let take_relevant (ass_list:tterm list) interesting_ids =
  let map = index_assumptions ass_list in
  let interesting_ids = String.Set.of_list interesting_ids in
  let rec take_relevant_impl interesting_ids processed =
    let relevant_asses =
      List.join (List.map (String.Set.to_list interesting_ids) ~f:(fun id ->
        match String.Map.find map id with
        | Some asses_list -> asses_list
        | None -> []))
    in
    let new_ids = String.Set.diff
        (String.Set.of_list (List.join (List.map relevant_asses ~f:(fun (_,ids) -> ids))))
        processed
    in
    let relevant_asses = (List.map relevant_asses ~f:(fun (ass,_) -> ass)) in
    if (String.Set.is_empty new_ids) then (relevant_asses,processed)
    else
      let (relevant_sttmts,relevant_ids) =
        take_relevant_impl new_ids (String.Set.union interesting_ids processed)
      in
      (relevant_sttmts@relevant_asses,relevant_ids)
  in
  let (relevant_sttmts,relevant_ids) =
    take_relevant_impl interesting_ids String.Set.empty
  in
  (List.dedup relevant_sttmts,String.Set.to_list relevant_ids)

(*---- here ----*)

(* let synonimize_term_in_tterms a b tterms =
  List.fold tterms ~init:tterms ~f:(fun acc tterm ->
      let acc =
        if tterm_contains_term tterm a then
          (replace_term_in_tterm a b tterm)::acc
        else acc
      in
      if tterm_contains_term tterm b then
        (replace_term_in_tterm b a tterm)::acc
      else acc)

let synonimize_by_equalities ass_list eqs =
  List.fold eqs ~init:ass_list
    ~f:(fun acc eq ->
         match eq.v with
         | Bop(Eq,lhs,rhs) ->
           synonimize_term_in_tterms lhs.v rhs.v acc
         | _ -> failwith "eqs must contain only equalities")
*)

let not_stronger op1 op2 =
  if op1=op2 then true else
    match op1,op2 with
    | Le,Lt -> true
    | Ge,Gt -> true
    | _,_ -> false

let prepare_assertions tip_res tip_ret_name tip_ret_type =
  let exists_such =
    (List.map tip_res.args_post_conditions
       ~f:(fun spec -> {v=Bop (Eq,{v=Id spec.name;t=Unknown},
                               spec.value);
                        t=Boolean})) @
    tip_res.post_statements
  in
  match tip_ret_name with
  | Some ret_name ->
    {v=Bop (Eq,{v=Id ret_name;t=tip_ret_type},
            tip_res.ret_val);t=Boolean} :: exists_such
  | None -> exists_such

let apply_assignments assignments ir =
  {ir with
   free_vars = List.fold assignments ~init:ir.free_vars
       ~f:(fun acc (name,value) ->
           let prev = String.Map.find_exn acc name in
           String.Map.add acc ~key:name
             ~data:{name;value={t=prev.value.t;v=value}})}

let apply_assignments_to_statements assignments statements =
  List.fold assignments ~init:statements
    ~f:(fun acc (name,value) ->
        replace_term_in_tterms (Id name) value acc)

let find_assignment assumptions assertions var =
  lprintf "searching for %s:{\n given:\n" var;
  List.iter assumptions ~f:(fun x -> lprintf "%s\n" (render_tterm x));
  lprintf "such that:\n";
  List.iter assertions ~f:(fun x -> lprintf "%s\n" (render_tterm x));
  let assignment =
    List.find_map assertions ~f:(fun assertion ->
        match assertion with
        | {v=Bop (Eq, {v=Id lhs;_}, rhs);_}
          when lhs = var ->
          Some (var,rhs.v)
        | {v=Bop (Eq, lhs, {v=Id rhs;_});_}
          when rhs = var ->
          Some (var,lhs.v)
        | {v=Bop (Le, {v=Int lhs;_}, {v=Id rhs;_});_}
          when rhs = var -> (* TODO: prioritize this assignments
                               less than the above ones*)
          lprintf "exploiting inequality: %d <= %s\n" lhs rhs;
          Some (var,Int lhs)
        | _ -> None)
  in
  begin
    lprintf "} ";
    match assignment with
    | Some (_,value) -> lprintf "found: %s = %s\n" var (render_term value)
    | None -> lprintf "NOTHING found\n"
  end;
  assignment

let guess_assignments assumptions assertions vars =
  List.join (List.map vars ~f:(fun var ->
      let (assertions,rel_ids) = take_relevant assertions [var] in
      let (assumptions,_) = take_relevant assumptions rel_ids in
      match find_assignment assumptions assertions var with
      | Some assignment -> [assignment]
      | None -> []))

let is_assignment_justified var value executions =
  lprintf "justifying assignment %s = %s\n" var (render_term value);
  let valid = List.for_all executions ~f:(fun assumptions ->
      let orig_assumptions = assumptions |> simplify [Id var; value] in
      let rel_assumptions = take_relevant orig_assumptions [var] |> fst |>
                            simplify [Id var]
      in
      let mod_assumptions =
        replace_term_in_tterms (Id var) value orig_assumptions in
      lprintf "comparing:\n";
      List.iter rel_assumptions ~f:(fun ass -> lprintf "%s\n" (render_tterm ass));
      lprintf "with\n";
      List.iter orig_assumptions ~f:(fun ass -> lprintf "%s\n" (render_tterm ass));
      List.for_all mod_assumptions ~f:(fun mod_assumption ->
          List.exists orig_assumptions
            ~f:(fun assumption ->
                term_eq assumption.v mod_assumption.v)))
  in
  lprintf "%s\n" (if valid then "justified" else "unjustified");
  valid

let induce_symbolic_assignments ir executions =
  start_log ();
  let fr_var_names = List.map (String.Map.data ir.free_vars) ~f:(fun spec -> spec.name) in
  lprintf "free vars: \n";
  lprint_list fr_var_names;
  let assertion_lists = List.map ir.tip_call.results ~f:(fun result ->
      (prepare_assertions result
         ir.tip_call.context.ret_name
         ir.tip_call.context.ret_type) |>
      canonicalize)
  in
  let assignments = List.fold assertion_lists ~init:[]
      ~f:(fun assignments assertions ->
          lprintf "working with assertions:\n";
          lprint_list (List.map assertions ~f:render_tterm);
          List.fold executions ~init:assignments
            ~f:(fun assignments assumptions ->
                let vars = String.Set.to_list
                    (String.Set.diff (String.Set.of_list fr_var_names)
                       (String.Set.of_list (List.map assignments ~f:fst)))
                in
                let var_ids = List.map vars ~f:(fun name -> Id name) in
                let assumptions = assumptions |> simplify var_ids
                in
                lprintf "\nassuming assignments:\n";
                lprint_list (List.map assignments
                               ~f:(fun a -> (fst a) ^ " = " ^
                                            (render_term (snd a))));
                lprintf "\nworking with assumptions:\n";
                lprint_list (List.map assumptions ~f:render_tterm);
                lprintf "\n|-|-|- vars:\n";
                lprint_list vars;
                assignments@(guess_assignments assumptions assertions vars)))
  in
  lprintf "\n\n JUSTIFYING \n\n";
  let justified_assignments = List.filter assignments ~f:(fun (var,value) ->
      is_assignment_justified var value executions)
  in
  apply_assignments justified_assignments ir;
