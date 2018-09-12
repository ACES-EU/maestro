#include "coherence.h"

/*@
  predicate dmap_dchain_coherent<t1,t2,vt>(dmap<t1,t2,vt> m, dchain ch) =
    dchain_index_range_fp(ch) == dmap_cap_fp(m) &*&
    true == subset(dchain_indexes_fp(ch), dmap_indexes_used_fp(m)) &*&
    true == subset(dmap_indexes_used_fp(m), dchain_indexes_fp(ch));
  @*/

/*@

lemma void coherent_same_indexes<t1,t2,vt>
             (dmap<t1,t2,vt> m, dchain ch)
requires dmap_dchain_coherent(m, ch);
ensures dmap_dchain_coherent(m, ch) &*&
        true == subset(dchain_indexes_fp(ch), dmap_indexes_used_fp(m)) &*&
        true == subset(dmap_indexes_used_fp(m), dchain_indexes_fp(ch));
{
  open dmap_dchain_coherent(m, ch);
  close dmap_dchain_coherent(m, ch);
}
@*/

/*@

lemma void empty_dmap_dchain_coherent<t1,t2,vt>(int len)
requires 0 <= len;
ensures dmap_dchain_coherent<t1,t2,vt>
         (empty_dmap_fp<t1,t2,vt>(len), empty_dchain_fp(len, 0));
{
  empty_dmap_cap<t1,t2,vt>(len);
  dmap_empty_no_indexes_used<t1,t2,vt>(len);
  close dmap_dchain_coherent(empty_dmap_fp<t1,t2,vt>(len),
                             empty_dchain_fp(len, 0));
}

lemma void coherent_dmap_used_dchain_allocated<t1,t2,vt>
             (dmap<t1,t2,vt> m, dchain ch, int idx)
requires dmap_dchain_coherent(m, ch) &*& dmap_index_used_fp(m, idx) == true;
ensures dmap_dchain_coherent(m, ch) &*&
        dchain_allocated_fp(ch, idx) == true;
{
  open dmap_dchain_coherent(m, ch);
  dmap_index_used_inbounds(m, idx);
  dmap_indexes_contain_index_used(m, idx);
  mem_subset(idx, dmap_indexes_used_fp(m), dchain_indexes_fp(ch));
  dchain_indexes_contain_index(ch, idx);
  close dmap_dchain_coherent(m, ch);
}

@*/

/*@
lemma void rejuvenate_preserves_coherent<t1,t2,vt>
             (dmap<t1,t2,vt> m, dchain ch,
              int index, time_t time)
requires dmap_dchain_coherent(m, ch) &*&
         true == dchain_allocated_fp(ch, index);
ensures dmap_dchain_coherent(m, dchain_rejuvenate_fp(ch, index, time));
{
  open dmap_dchain_coherent(m, ch);
  dchain_rejuvenate_preserves_indexes_set(ch, index, time);
  rejuvenate_preserves_index_range(ch, index, time);
  dchain nch = dchain_rejuvenate_fp(ch, index, time);
  subset_trans(dchain_indexes_fp(nch), dchain_indexes_fp(ch),
               dmap_indexes_used_fp(m));
  subset_trans(dmap_indexes_used_fp(m), dchain_indexes_fp(ch),
               dchain_indexes_fp(nch));
  close dmap_dchain_coherent(m, nch);
}
@*/

/*@
  lemma void dmap_put_equiv_indexes_sub<vt>(list<option<vt> > vals,
                                            int ind, vt v, int start)
  requires true;
  ensures true == subset(nonempty_indexes_fp(update(ind-start, some(v), vals),
                                             start),
                         cons(ind, nonempty_indexes_fp(vals, start)));
  {
    switch(vals) {
      case nil: return;
      case cons(h,t):
        if (start == ind) {
          switch(h) {
            case none: break;
            case some(lll):
              add_extra_preserves_subset(nonempty_indexes_fp(t, start+1),
                                         nonempty_indexes_fp(t, start+1), start);
          }
          add_extra_preserves_subset(nonempty_indexes_fp(t, start+1),
                                     nonempty_indexes_fp(vals, start), ind);
        } else {
          dmap_put_equiv_indexes_sub(t, ind, v, start+1);
          switch(h) {
            case none: break;
            case some(aaa):
              list<int> prev_idxes = nonempty_indexes_fp(t, start+1);
              add_extra_preserves_subset(prev_idxes, prev_idxes, start);
              add_extra_preserves_subset(prev_idxes, cons(start, prev_idxes),
                                         ind);

              subset_trans(nonempty_indexes_fp(update(ind-start-1, some(v), t),
                                               start+1),
                           cons(ind, nonempty_indexes_fp(t, start+1)),
                           cons(ind, nonempty_indexes_fp(vals, start)));
              break;
          }
        }
    }
  }
  @*/

/*@
  lemma void dmap_put_equiv_indexes_sup<vt>(list<option<vt> > vals,
                                            int ind, vt v, int start)
  requires true;
  ensures true == subset(nonempty_indexes_fp(vals, start),
                         nonempty_indexes_fp(update(ind-start, some(v), vals),
                                             start));
  {
    switch(vals) {
      case nil: return;
      case cons(h,t):
        dmap_put_equiv_indexes_sup(t, ind, v, start+1);
        if (ind == start) {
          add_extra_preserves_subset(nonempty_indexes_fp(t, start+1),
                                     nonempty_indexes_fp(t, start+1),
                                     start);
        }
        switch(h) {
          case none:
            return;
          case some(ignore):
            add_extra_preserves_subset(nonempty_indexes_fp(t, start+1),
                                       nonempty_indexes_fp(update(ind-start-1,
                                                                  some(v), t),
                                                           start+1),
                                       start);
            break;
        }
    }
  }
  @*/

/*@
  lemma void dmap_put_occupies<vt>(list<option<vt> > vals,
                                   int ind, vt v, int start)
  requires 0 <= start &*& start <= ind &*& ind - start < length(vals);
  ensures true == mem(ind, nonempty_indexes_fp(update(ind-start, some(v), vals),
                                               start));
  {
    switch(vals) {
      case nil: return;
      case cons(h,t):
        if (start == ind) return;
        dmap_put_occupies(t, ind, v, start+1);
        switch(h) {
          case none: break;
          case some(ignore): break;
        }
    }
  }
  @*/

/*@
  lemma void dmap_put_equiv_indexes<t1,t2,vt>(dmap<t1,t2,vt> m,
                                              int ind, vt value,
                                              fixpoint (vt,t1) vk1,
                                              fixpoint (vt,t2) vk2)
  requires 0 <= ind &*& ind < dmap_cap_fp(m);
  ensures true == subset(dmap_indexes_used_fp
                          (dmap_put_fp(m, ind, value, vk1, vk2)),
                         cons(ind, dmap_indexes_used_fp(m))) &*&
          true == subset(cons(ind, dmap_indexes_used_fp(m)),
                         dmap_indexes_used_fp
                          (dmap_put_fp(m, ind, value, vk1, vk2)));
  {
    switch(m) { case dmap(ma, mb, vals):
      dmap_put_equiv_indexes_sub(vals, ind, value, 0);
      dmap_put_equiv_indexes_sup(vals, ind, value, 0);
      dmap_put_occupies(vals, ind, value, 0);
    }
  }
  @*/

/*@
lemma void coherent_put_allocated_preserves_coherent<t1,t2,vt>
(dmap<t1,t2,vt> m, dchain ch, t1 k1, t2 k2,
 vt value, int ind, time_t t,
 fixpoint (vt,t1) vk1,
 fixpoint (vt,t2) vk2)
requires dmap_dchain_coherent(m, ch) &*&
         0 <= ind &*& ind < dmap_cap_fp(m);
ensures dmap_dchain_coherent(dmap_put_fp(m, ind, value, vk1, vk2),
                             dchain_allocate_fp(ch, ind, t));
{
  open dmap_dchain_coherent(m, ch);
  dchain_allocate_append_to_indexes(ch, ind, t);
  assert dchain_indexes_fp(dchain_allocate_fp(ch, ind, t)) ==
         append(dchain_indexes_fp(ch), cons(ind, nil));
  if (mem(ind, dmap_indexes_used_fp(m))) {
    subset_mem_trans(dmap_indexes_used_fp(m), dchain_indexes_fp(ch), ind);
  }
  dmap_put_equiv_indexes(m, ind, value, vk1, vk2);
  assert true == subset(dmap_indexes_used_fp(dmap_put_fp(m, ind, value,
                                                         vk1, vk2)),
                        cons(ind, dmap_indexes_used_fp(m)));
  assert true == subset(dmap_indexes_used_fp(m),
                        dmap_indexes_used_fp(dmap_put_fp(m, ind, value,
                                                         vk1, vk2)));

  dmap_put_preserves_cap(m, ind, value, vk1, vk2);
  allocate_preserves_index_range(ch, ind, t);
  subset_append2(dmap_indexes_used_fp(m), dchain_indexes_fp(ch),
                 cons(ind, nil));
  add_extra_preserves_subset(dchain_indexes_fp(ch),
                             dmap_indexes_used_fp(m), ind);
  subset_append(dchain_indexes_fp(ch), cons(ind, nil),
                cons(ind, dmap_indexes_used_fp(m)));
  subset_trans(dmap_indexes_used_fp(dmap_put_fp(m, ind, value, vk1, vk2)),
               cons(ind, dmap_indexes_used_fp(m)),
               dchain_indexes_fp(dchain_allocate_fp(ch, ind, t)));
  subset_trans(dchain_indexes_fp(dchain_allocate_fp(ch, ind, t)),
               cons(ind, dmap_indexes_used_fp(m)),
               dmap_indexes_used_fp(dmap_put_fp(m, ind, value, vk1, vk2)));
  close dmap_dchain_coherent(dmap_put_fp(m, ind, value, vk1, vk2),
                             dchain_allocate_fp(ch, ind, t));
}

@*/


/*@
  lemma void dchain_out_of_space_to_indexes_size(dchain ch)
  requires true;
  ensures dchain_out_of_space_fp(ch) ==
          (dchain_index_range_fp(ch) <= length(dchain_indexes_fp(ch)));
  {
    switch(ch) { case dchain(alist, index_range, lo, hi):
      map_effect_on_len(alist, fst);
    }
  }
  @*/

/*@

lemma void coherent_dchain_non_out_of_space_map_nonfull<t1,t2,vt>
            (dmap<t1,t2,vt> m, dchain ch)
requires dmappingp<t1,t2,vt>(m, ?a, ?b, ?c, ?d, ?e, ?g, ?h, ?i, ?j, ?k, ?l, ?n, ?f) &*&
         dmap_dchain_coherent(m, ch) &*&
         dchain_out_of_space_fp(ch) == false;
ensures dmappingp<t1,t2,vt>(m, a, b, c, d, e, g, h, i, j, k, l, n, f) &*&
        dmap_dchain_coherent(m, ch) &*&
        dmap_size_fp(m) < dmap_cap_fp(m);
{
  open dmap_dchain_coherent(m, ch);

  dmap_indexes_used_distinct(m);
  distinct_subset_sublen(dmap_indexes_used_fp(m), dchain_indexes_fp(ch));
  dchain_out_of_space_to_indexes_size(ch);
  dmap_size_of_indexes_used(m);

  close dmap_dchain_coherent(m, ch);
}

@*/

/*@
  lemma void coherent_expire_one<t1,t2,vt>(dmap<t1,t2,vt> m,
                                           dchain ch,
                                           int idx,
                                           fixpoint (vt,t1) vk1,
                                           fixpoint (vt,t2) vk2)
  requires dmap_dchain_coherent(m, ch) &*&
           dchain_nodups(ch) &*&
           true == dchain_allocated_fp(ch, idx) &*&
           0 <= idx;
  ensures dmap_dchain_coherent(dmap_erase_fp(m, idx, vk1, vk2),
                               dchain_remove_index_fp(ch, idx)) &*&
          dchain_nodups(dchain_remove_index_fp(ch, idx));
  {
    open dmap_dchain_coherent(m, ch);
    dmap<t1,t2,vt> nm = dmap_erase_fp(m, idx, vk1, vk2);
    dchain nch = dchain_remove_index_fp(ch, idx);
    dchain_remove_keeps_ir(ch, idx);
    dmap_erase_keeps_cap(m, idx, vk1, vk2);
    assert dchain_index_range_fp(nch) == dmap_cap_fp(nm);
    dchain_remove_idx_from_indexes(ch, idx);
    assert dchain_indexes_fp(nch) ==
           remove(idx, dchain_indexes_fp(ch));
    dmap_erase_removes_index(m, idx, vk1, vk2);
    assert dmap_indexes_used_fp(nm) ==
           remove(idx, dmap_indexes_used_fp(m));

    dchain_nodups_unique_idx(ch, idx);
    dmap_indexes_used_distinct(m);
    distinct_mem_remove(idx, dmap_indexes_used_fp(m));
    remove_both_subset(idx, dchain_indexes_fp(ch), dmap_indexes_used_fp(m));
    remove_both_subset(idx, dmap_indexes_used_fp(m), dchain_indexes_fp(ch));

    dchain_remove_keeps_nodups(ch, idx);

    close dmap_dchain_coherent(nm, nch);
  }
  @*/

/*@
  lemma void coherent_same_cap<t1,t2,vt>(dmap<t1,t2,vt> m, dchain ch)
  requires dmap_dchain_coherent(m, ch);
  ensures dmap_dchain_coherent(m, ch) &*&
          dmap_cap_fp(m) == dchain_index_range_fp(ch);
  {
    open dmap_dchain_coherent(m, ch);
    close dmap_dchain_coherent(m, ch);
  }
  @*/

/*@
  lemma void coherent_old_index_used<t1,t2,vt>(dmap<t1,t2,vt> m, dchain ch)
  requires dmap_dchain_coherent(m, ch) &*&
           false == dchain_is_empty_fp(ch) &*&
           0 <= dchain_get_oldest_index_fp(ch) &*&
           dchain_get_oldest_index_fp(ch) < dchain_index_range_fp(ch);
  ensures dmap_dchain_coherent(m, ch) &*&
          true == dmap_index_used_fp(m, dchain_get_oldest_index_fp(ch));
  {
    dchain_oldest_allocated(ch);
    coherent_same_cap(m, ch);
    open dmap_dchain_coherent(m, ch);
    dchain_indexes_contain_index(ch, dchain_get_oldest_index_fp(ch));
    mem_subset(dchain_get_oldest_index_fp(ch), dchain_indexes_fp(ch),
               dmap_indexes_used_fp(m));
    dmap_indexes_contain_index_used(m, dchain_get_oldest_index_fp(ch));
    close dmap_dchain_coherent(m, ch);
  }
  @*/

/*@
  lemma void kkeeper_erase_one_from_vec<t>(list<void*> addrs,
                                           list<pair<t, real> > contents,
                                           list<pair<t, void*> > addr_map,
                                           int index)
  requires 0 <= index &*& index < length(contents) &*&
           true == forall2(contents, addrs, (kkeeper)(addr_map));
  ensures true == forall2(vector_erase_fp(contents, index), addrs,
                          (kkeeper)(addr_map));
  {
    switch(contents) {
      case nil: return;
      case cons(ch, ct):
        switch(addrs) {
          case nil: return;
          case cons(ah, at):
            if (index == 0) return;
            kkeeper_erase_one_from_vec(at, ct, addr_map, index - 1);
      }
    }
  }

  fixpoint bool owned_or_not_this<t>(t val, pair<t, real> cell) {
    return snd(cell) == 1.0 || fst(cell) != val;
  }

  lemma void kkeeper_erase_one_from_map<t>(list<void*> addrs,
                                           list<pair<t, real> > contents,
                                           list<pair<t, void*> > addr_map,
                                           t val)
  requires true == forall2(contents, addrs, (kkeeper)(addr_map)) &*&
           true == forall(contents, (owned_or_not_this)(val));
  ensures true == forall2(contents, addrs,
                          (kkeeper)(map_erase_fp(addr_map, val)));
  {
    switch(contents) {
      case nil: return;
      case cons(ch, ct):
        switch(addrs) {
          case nil: return;
          case cons(ah, at):
            if (snd(ch) != 1.0) {
              map_erase_keeps_others(addr_map, val, fst(ch));
            }
            kkeeper_erase_one_from_map(at, ct, addr_map, val);
      }
    }
  }

  lemma void kkeeper_nth_addrs_is_map_get<t>(list<void*> addrs,
                                             list<pair<t, real> > contents,
                                             list<pair<t, void*> > addr_map,
                                             int index)
  requires 0 <= index &*& index < length(contents) &*&
           length(contents) <= length(addrs) &*&
           true == forall2(contents, addrs, (kkeeper)(addr_map)) &*&
           nth(index, contents) == pair(?val, ?frac) &*&
           frac != 1.0;
  ensures map_get_fp(addr_map, val) == nth(index, addrs);
  {
    switch(contents) {
      case nil: return;
      case cons(ch, ct):
        switch(addrs) {
          case nil: return;
          case cons(ah, at):
            if (index == 0) {
              return;
            }
            kkeeper_nth_addrs_is_map_get(at, ct, addr_map, index - 1);
        }
     }
  }

  lemma void kkeeper_non_mem_non_mem<t>(list<void*> addrs,
                                        list<pair<t, real> > contents,
                                        list<pair<t, void*> > addr_map,
                                        t val)
  requires true == forall2(contents, addrs, (kkeeper)(addr_map)) &*&
           length(contents) <= length(addrs) &*&
           false == mem(map_get_fp(addr_map, val), addrs);
  ensures true == forall(contents, (owned_or_not_this)(val));
  {
    switch(contents) {
      case nil: return;
      case cons(ch, ct):
        switch(addrs) {
          case nil: return;
          case cons(ah, at):
            kkeeper_non_mem_non_mem(at, ct, addr_map, val);
      }
    }
  }

  lemma void kkeeper_no_dups_owned_or_not_this<t>(list<void*> addrs,
                                                  list<pair<t, real> > contents,
                                                  list<pair<t, void*> > addr_map,
                                                  int index)
  requires 0 <= index &*& index < length(contents) &*&
           length(contents) <= length(addrs) &*&
           true == forall2(contents, addrs, (kkeeper)(addr_map)) &*&
           nth(index, contents) == pair(?val, ?frac) &*&
           frac != 1.0 &*&
           true == no_dups(addrs);
  ensures true == forall(vector_erase_fp(contents, index),
                         (owned_or_not_this)(val));
  {
    switch(contents) {
      case nil: return;
      case cons(ch, ct):
        switch(addrs) {
          case nil: return;
          case cons(ah, at):
            if (index == 0) {
              assert false == mem(ah, at);
              assert map_get_fp(addr_map, val) == ah;
              kkeeper_non_mem_non_mem(at, ct, addr_map, val);
              assert true == forall(ct, (owned_or_not_this)(val));
              return;
            }
            kkeeper_no_dups_owned_or_not_this(at, ct, addr_map, index - 1);
            if (snd(ch) != 1.0) {
              if (fst(ch) == fst(nth(index, contents))) {
                kkeeper_nth_addrs_is_map_get(addrs, contents, addr_map, index);
                assert ah == nth(index, addrs);
                assert true == mem(ah, addrs);
                assert false;
              }
            }
      }
    }
  }

  lemma void kkeeper_erase_one<t>(list<void*> addrs,
                                  list<pair<t, real> > contents,
                                  list<pair<t, void*> > addr_map,
                                  int index)
  requires 0 <= index &*& index < length(contents) &*&
           length(contents) <= length(addrs) &*&
           true == forall2(contents, addrs, (kkeeper)(addr_map)) &*&
           nth(index, contents) == pair(?val, ?frac) &*&
           frac != 1.0 &*&
           true == no_dups(addrs);
  ensures true == forall2(vector_erase_fp(contents, index), addrs,
                          (kkeeper)(map_erase_fp(addr_map, val)));
  {
    kkeeper_erase_one_from_vec(addrs, contents, addr_map, index);
    nth_update(index, index, pair(val, 1.0), contents);
    kkeeper_no_dups_owned_or_not_this(addrs, contents, addr_map, index);
    kkeeper_erase_one_from_map(addrs, vector_erase_fp(contents, index),
                               addr_map, val);
  }
  @*/

/*@
  lemma void empty_kkeeper<t>(list<void*> addrs,
                              list<pair<t, real> > contents,
                              list<pair<t, void*> > addr_map,
                              int capacity)
  requires length(contents) == capacity &*&
           true == forall(contents, is_one);
  ensures true == forall2(contents, addrs, (kkeeper)(addr_map));
  {
    switch(contents) {
      case nil: return;
      case cons(h1,t1):
        switch(addrs) {
          case nil: return;
            case cons(h2,t2):
              empty_kkeeper(t2, t1, addr_map, capacity - 1);
        }
    }
  }
  @*/

/*@
  fixpoint bool consistent_pair<kt>(list<pair<kt, int> > m,
                                    dchain ch,
                                    int idx, pair<kt, real> el) {
    switch(el) {
      case pair(car, cdr):
        return cdr == 1.0 ?
          (false == dchain_allocated_fp(ch, idx))
          :
          (cdr == 0.75 &&
           map_has_fp(m, car) &&
           map_get_fp(m, car) == idx &&
           dchain_allocated_fp(ch, idx));
    }
  }
  @*/

/*@
  fixpoint bool engaged_cell<kt>(pair<kt, real> p) {
    return snd(p) != 1.0;
  }
  @*/

/*@
  predicate map_vec_chain_coherent<kt>(list<pair<kt, int> > m,
                                       list<pair<kt, real> > v,
                                       dchain ch) =
    dchain_index_range_fp(ch) == length(v) &*&
    true == forall_idx(v, 0, (consistent_pair)(m, ch)) &*&
    true == msubset(map(fst, m), map(fst, filter(engaged_cell, v)));
  @*/

/*@
  lemma void mvc_coherent_bounds<kt>(list<pair<kt, int> > m,
                                     list<pair<kt, real> > v,
                                     dchain ch)
  requires map_vec_chain_coherent<kt>(m, v, ch);
  ensures dchain_index_range_fp(ch) == length(v) &*&
          map_vec_chain_coherent<kt>(m, v, ch);
  {
    open map_vec_chain_coherent(m, v, ch);
    close map_vec_chain_coherent(m, v, ch);
  }
  @*/

/*@
  lemma void mvc_coherent_index_busy<kt>(list<pair<kt, int> > m,
                                         list<pair<kt, real> > v,
                                         dchain ch,
                                         uint32_t index)
  requires map_vec_chain_coherent<kt>(m, v, ch) &*&
           true == dchain_allocated_fp(ch, index) &*&
           0 <= index &*& index < dchain_index_range_fp(ch);
  ensures map_vec_chain_coherent<kt>(m, v, ch) &*&
          nth(index, v) == pair(?key, ?frac) &*&
          frac == 0.75 &*&
          true == map_has_fp(m, key) &*&
          map_get_fp(m, key) == index;
  {
    mvc_coherent_bounds(m, v, ch);
    open map_vec_chain_coherent(m, v, ch);
    extract_prop_by_idx(v, (consistent_pair)(m, ch), 0, index);
    pair<kt, real> p = nth(index, v);
    switch(p) {
      case pair(car, cdr):
        assert true == map_has_fp(m, car);
        assert map_get_fp(m, car) == index;
        assert true == dchain_allocated_fp(ch, index);
        assert cdr == 0.75;
    }
    close map_vec_chain_coherent(m, v, ch);
  }
  @*/

/*@
  lemma int pairs_consistent_get_index<kt>(list<pair<kt, int> > m,
                                           list<pair<kt, real> > v,
                                           dchain ch,
                                           kt k, int start_idx)
  requires true == forall_idx(v, start_idx, (consistent_pair)(m, ch)) &*&
           true == map_has_fp(m, k) &*&
           true == mem(k, map(fst, filter(engaged_cell, v)));
  ensures 0 <= result - start_idx &*& result - start_idx < length(v) &*&
          true == consistent_pair(m, ch, result, nth(result - start_idx, v)) &*&
          true == dchain_allocated_fp(ch, result) &*&
          result == map_get_fp(m, k) &*&
          1.0 != snd(nth(result - start_idx, v));
  {
    switch(v) {
      case nil:
        return 0;
      case cons(h, t):
        switch(h) { case pair(car, cdr):
            if (car == k && cdr != 1.0) {
              return start_idx;
            } else {
              return pairs_consistent_get_index(m, t, ch, k, start_idx + 1);
            }
        }
    }
  }
  @*/

/*@
  lemma void mvc_coherent_map_get_bounded<kt>(list<pair<kt, int> > m,
                                              list<pair<kt, real> > v,
                                              dchain ch,
                                              kt k)
  requires map_vec_chain_coherent<kt>(m, v, ch) &*&
           true == map_has_fp(m, k);
  ensures 0 <= map_get_fp(m, k) &*& map_get_fp(m, k) < length(v) &*&
          dchain_index_range_fp(ch) == length(v) &*&
          map_vec_chain_coherent<kt>(m, v, ch) &*&
          true == dchain_allocated_fp(ch, map_get_fp(m, k)) &*&
          1.0 != snd(nth(map_get_fp(m, k), v));
  {
    mvc_coherent_bounds(m, v, ch);
    open map_vec_chain_coherent(m, v, ch);
    map_has_to_mem(m, k);
    msubset_subset(map(fst, m), map(fst, filter(engaged_cell, v)));
    subset_mem_trans(map(fst, m), map(fst, filter(engaged_cell, v)), k);
    pairs_consistent_get_index(m, v, ch, k, 0);
    close map_vec_chain_coherent(m, v, ch);
  }
  @*/

/*@
  lemma void mvc_coherent_map_get_vec_half<kt>(list<pair<kt, int> > m,
                                               list<pair<kt, real> > v,
                                               dchain ch,
                                               kt k)
  requires map_vec_chain_coherent<kt>(m, v, ch) &*&
           true == map_has_fp(m, k);
  ensures 0 <= map_get_fp(m, k) &*& map_get_fp(m, k) < length(v) &*&
          dchain_index_range_fp(ch) == length(v) &*&
          map_vec_chain_coherent<kt>(m, v, ch) &*&
          true == dchain_allocated_fp(ch, map_get_fp(m, k)) &*&
          1.0 != snd(nth(map_get_fp(m, k), v));
  {
    mvc_coherent_map_get_bounded(m, v, ch, k);
  }
  @*/

/*@
  lemma void rejuvenate_pairs_still_consistent<kt>(list<pair<kt, int> > m,
                                                   list<pair<kt, real> > v,
                                                   dchain ch,
                                                   int index, time_t time,
                                                   int start_idx)
  requires true == forall_idx(v, start_idx, (consistent_pair)(m, ch)) &*&
           true == dchain_allocated_fp(ch, index);
  ensures true == forall_idx(v, start_idx, (consistent_pair)(m, dchain_rejuvenate_fp(ch, index, time)));
  {
    switch(v) {
      case nil:
      case cons(h, t):
        switch(h) {
          case pair(car, cdr):
            dchain_rejuvenate_preserves_indexes_set(ch, index, time);
            dchain nch = dchain_rejuvenate_fp(ch, index, time);
            dchain_indexes_contain_index(ch, start_idx);
            dchain_indexes_contain_index(nch, start_idx);
            if (cdr != 1.0) {
              subset_mem_trans(dchain_indexes_fp(ch), dchain_indexes_fp(nch), start_idx);
            } else {
              if (dchain_allocated_fp(nch, start_idx))
                subset_mem_trans(dchain_indexes_fp(nch), dchain_indexes_fp(ch), start_idx);
            }
            rejuvenate_pairs_still_consistent(m, t, ch, index, time, start_idx + 1);
        }
    }
  }
  @*/

/*@
  lemma void mvc_rejuvenate_preserves_coherent<kt>(list<pair<kt, int> > m,
                                                   list<pair<kt, real> > v,
                                                   dchain ch,
                                                   int index, time_t time)
  requires map_vec_chain_coherent<kt>(m, v, ch) &*&
           true == dchain_allocated_fp(ch, index);
  ensures map_vec_chain_coherent<kt>(m, v, dchain_rejuvenate_fp(ch,
                                                                index,
                                                                time));
  {
    open map_vec_chain_coherent(m, v, ch);
    rejuvenate_pairs_still_consistent(m, v, ch, index, time, 0);
    rejuvenate_preserves_index_range(ch, index, time);
    close map_vec_chain_coherent(m, v, dchain_rejuvenate_fp(ch, index, time));
  }
  @*/

/*@
  lemma void mvc_coherent_alloc_is_halfowned<kt>(list<pair<kt, int> > m,
                                                 list<pair<kt, real> > v,
                                                 dchain ch,
                                                 int index)
  requires map_vec_chain_coherent<kt>(m, v, ch) &*&
           0 <= index &*& index < length(v);
  ensures map_vec_chain_coherent<kt>(m, v, ch) &*&
          (snd(nth(index, v)) == 1.0) != dchain_allocated_fp(ch, index);
  {
    open map_vec_chain_coherent(m, v, ch);
    extract_prop_by_idx(v, (consistent_pair)(m, ch), 0, index);
    switch(nth(index, v)) {case pair(car, cdr):}
    close map_vec_chain_coherent(m, v, ch);
  }
  @*/

/*@
  lemma void consistent_pairs_no_key<kt>(list<pair<kt, int> > m,
                                         list<pair<kt, real> > v,
                                         dchain ch,
                                         kt key,
                                         int start_idx)
  requires true == forall_idx(v, start_idx, (consistent_pair)(m, ch)) &*&
           false == map_has_fp(m, key);
  ensures true == forall(v, (is_one_if_equals)(key));
  {
    switch(v) {
      case nil:
      case cons(h, t):
        if (fst(h) == key) {
          assert true == consistent_pair(m, ch, start_idx, h);
          // Yes, VeriFast actually requires inlining the very thing we just asserted :(
          switch(h) { case pair(car, cdr):
            assert cdr == 1.0 ? (false == dchain_allocated_fp(ch, start_idx))
                              : (cdr == 0.75 && map_has_fp(m, car) &&
                                 map_get_fp(m, car) == start_idx && dchain_allocated_fp(ch, start_idx));
          }
          assert false == map_has_fp(m, fst(h));
          assert snd(h) == 1.0;
          assert true == is_one_if_equals(key, h);
        }
        consistent_pairs_no_key(m, t, ch, key, start_idx + 1);
    }
  }
  @*/

/*@
  lemma void mvc_coherent_key_abscent<kt>(list<pair<kt, int> > m,
                                          list<pair<kt, real> > v,
                                          dchain ch,
                                          kt key)
  requires map_vec_chain_coherent<kt>(m, v, ch) &*&
           false == map_has_fp(m, key);
  ensures map_vec_chain_coherent<kt>(m, v, ch) &*&
          true == forall(v, (is_one_if_equals)(key));
  {
    mvc_coherent_bounds(m, v, ch);
    open map_vec_chain_coherent(m, v, ch);
    consistent_pairs_no_key(m, v, ch, key, 0);
    close map_vec_chain_coherent(m, v, ch);
  }
  @*/

/*@
  lemma void mvc_coherent_same_len<kt>(list<pair<kt, int> > m,
                                       list<pair<kt, real> > v,
                                       dchain ch)
  requires map_vec_chain_coherent<kt>(m, v, ch);
  ensures map_vec_chain_coherent<kt>(m, v, ch) &*&
          length(v) == dchain_index_range_fp(ch);
  {
    open map_vec_chain_coherent(m, v, ch);
    close map_vec_chain_coherent(m, v, ch);
  }
  @*/

/*@
  lemma void kkeeper_add_unrelevant<t>(list<void*> addrs,
                                       list<pair<t, real> > contents,
                                       list<pair<t, void*> > addr_map,
                                       t v,
                                       void* addr)
  requires true == forall2(contents, addrs, (kkeeper)(addr_map)) &*&
           true == forall(contents, (is_one_if_equals)(v));
  ensures true == forall2(contents, addrs, (kkeeper)(map_put_fp(addr_map, v, addr)));
  {
    switch(contents) {
      case nil:
      case cons(cnt_h, cnt_t):
        switch(addrs) {
          case nil:
          case cons(addr_h, addr_t):
            switch(cnt_h) { case pair(car,cdr):}
            kkeeper_add_unrelevant(addr_t, cnt_t, addr_map, v, addr);
        }
    }
  }
  @*/

/*@
  lemma void kkeeper_add_one<t>(list<void*> addrs,
                                list<pair<t, real> > contents,
                                list<pair<t, void*> > addr_map,
                                t v,
                                int index)
  requires 0 <= index &*& index < length(contents) &*&
           true == forall2(contents, addrs, (kkeeper)(addr_map)) &*&
           true == forall(contents, (is_one_if_equals)(v));
  ensures true == forall2(update(index, pair(v, 0.75), contents),
                          addrs,
                          (kkeeper)(map_put_fp(addr_map, v,
                                               nth(index, addrs))));
  {
     switch(contents) {
       case nil:
       case cons(cnt_h, cnt_t):
         switch(addrs) {
           case nil:
           case cons(addr_h, addr_t):
             switch(cnt_h) { case pair(fff,sss):}
             if (index == 0) {
               kkeeper_add_unrelevant(addr_t, cnt_t, addr_map, v, nth(index, addrs));
             } else {
               kkeeper_add_one(addr_t, cnt_t, addr_map, v, index - 1);
             }
         }
     }
  }
  @*/

/*@
  lemma void empty_map_vec_dchain_consistent_pairs<kt>(list<pair<kt, real> > vec, int len, int start_idx)
  requires true == forall(vec, is_one);
  ensures true == forall_idx(vec, start_idx, (consistent_pair)(nil, empty_dchain_fp(len, 0)));
  {
    switch(vec) {
      case nil:
      case cons(h, t):
        switch(h) {case pair(car, cdr):}
        empty_map_vec_dchain_consistent_pairs(t, len, start_idx + 1);
    }
  }
  @*/

/*@
  lemma void empty_map_vec_dchain_coherent<kt>(list<pair<kt, real> > vec)
  requires true == forall(vec, is_one);
  ensures map_vec_chain_coherent<kt>(nil, vec,
                                     empty_dchain_fp(length(vec), 0));
  {
    empty_map_vec_dchain_consistent_pairs(vec, length(vec), 0);
    close map_vec_chain_coherent<kt>(nil, vec, empty_dchain_fp(length(vec), 0));
  }
  @*/

/*@
  lemma void filter_engaged_len<kt>(list<pair<kt, int> > m,
                                    list<pair<kt, real> > v,
                                    dchain ch, int start_idx)
  requires true == forall_idx(v, start_idx, (consistent_pair)(m, ch));
          //&*& length(dchain_indexes_fp(ch)) <= length(v);
  ensures length(filter(engaged_cell, v)) == length(dchain_indexes_fp(ch));
  {
    assume(false);//TODO
    switch(v) {
      case nil:
      case cons(h, t):
        switch(h) { case pair(car,cdr):
          if (!cdr) {
            assume(true == forall_idx(v, start_idx, (consistent_pair)(m, dchain_remove_index_fp(ch, start_idx))));
            dchain_remove_idx_from_indexes(ch, start_idx);
            dchain_indexes_contain_index(ch, start_idx);
            assert length(dchain_indexes_fp(dchain_remove_index_fp(ch, start_idx))) <
                   length(dchain_indexes_fp(ch));
            filter_engaged_len(m, t, dchain_remove_index_fp(ch, start_idx), start_idx + 1);
          } else {
            filter_engaged_len(m, t, ch, start_idx + 1);
          }
        }
    }
  }
  @*/

/*@
  lemma void mvc_coherent_dchain_non_out_of_space_map_nonfull<kt>(list<pair<kt, int> > m,
                                                                  list<pair<kt, real> > v,
                                                                  dchain ch)
  requires map_vec_chain_coherent<kt>(m, v, ch) &*&
           dchain_out_of_space_fp(ch) == false;
  ensures map_vec_chain_coherent<kt>(m, v, ch) &*&
          map_size_fp(m) < dchain_index_range_fp(ch);
  {
    open map_vec_chain_coherent(m, v, ch);
    msubset_length(map(fst, m), map(fst, filter(engaged_cell, v)));
    map_preserves_length(fst, m);
    assert length(m) == length(map(fst, m));
    assert length(m) <= length(map(fst, filter(engaged_cell, v)));
    map_preserves_length(fst, filter(engaged_cell, v));
    assert length(map(fst, filter(engaged_cell, v))) <= length(filter(engaged_cell, v));
    filter_engaged_len(m, v, ch, 0);
    dchain_out_of_space_to_indexes_size(ch);
    assert length(filter(engaged_cell, v)) < length(v);
    close map_vec_chain_coherent(m, v, ch);
  }
  @*/

/*@
  lemma void mvc_coherent_put<kt>(list<pair<kt, int> > m,
                                  list<pair<kt, real> > v,
                                  dchain ch,
                                  int index, time_t time,
                                  kt key)
  requires map_vec_chain_coherent<kt>(m, v, ch) &*&
           false == dchain_allocated_fp(ch, index);
  ensures map_vec_chain_coherent<kt>(map_put_fp(m, key, index),
                                     update(index, pair(key, 0.75), v),
                                     dchain_allocate_fp(ch, index, time));
  {
    assume(false);//TODO
  }
  @*/

/*@
  lemma void mvc_coherent_expire_one<kt>(list<pair<kt, int> > m,
                                         list<pair<kt, real> > v,
                                         dchain ch,
                                         int index,
                                         kt key)
  requires map_vec_chain_coherent<kt>(m, v, ch) &*&
           nth(index, v) != pair(key, 1.0);
  ensures map_vec_chain_coherent<kt>(map_erase_fp(m, key),
                                     vector_erase_fp(v, index),
                                     dchain_remove_index_fp(ch, index));
  {
    assume(false);//TODO
  }
  @*/
