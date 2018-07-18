open Core

type verification_outcome =
  | Valid
  | Inconsistent
  | Invalid_seq
  | Invalid_rez of string

let verify_ir ir verifast fname outf proj_root lino_fname =
  Render.render_ir ir fname ~render_assertions:false;
  let _ = Sys.command (verifast ^ " -c -prover z3v4.5 -I " ^ proj_root ^ " -I ../nf/lib/stubs/dpdk -disable_overflow_check" ^
                       " " ^ fname ^ " > " ^ outf)
  in
  let vf_succeded = Sys.command ("grep '0 errors found' " ^ outf ^
                                 " > /dev/null")
  in
  if vf_succeded <> 0 then Invalid_seq
  else begin
    Render.render_ir ir fname ~render_assertions:true;
    let _ = Sys.command (verifast ^ " -c -prover z3v4.5 -I " ^ proj_root ^ " -I ../nf/lib/stubs/dpdk -disable_overflow_check" ^
                         " " ^ fname ^ " > " ^ outf)
    in
    let vf_succeded = Sys.command ("grep '0 errors found' " ^ outf ^
                                   " > /dev/null") in
    if vf_succeded = 0 then begin
      let _ = (* locate the line to dump VeriFast assumptions *)
        Sys.command ("sed -n '/" ^ ir.Ir.export_point ^ "/=' " ^
                     fname ^ " > " ^ lino_fname)
      in
      let export_lino = String.strip (In_channel.read_all lino_fname) in
      let _ = Sys.command (verifast ^ " -c -prover z3v4.5 -I " ^ proj_root ^ " -I ../nf/lib/stubs/dpdk -disable_overflow_check" ^
                           " -breakpoint " ^ export_lino ^
                           " " ^ fname ^ " > " ^ outf)
      in
      let consistent = Sys.command ("grep 'Breakpoint reached' " ^ outf ^
                                    " > /dev/null")
      in
      if consistent = 0 then Valid
      else Inconsistent
    end
    else begin
      let unproven_assertion =
        Sys.command ("grep 'Assertion might not hold' " ^ outf ^ " > /dev/null")
      in
      let err = In_channel.read_all outf in
      if unproven_assertion = 0 then Invalid_rez "Can not prove assertion"
      else Invalid_rez err
    end
  end
