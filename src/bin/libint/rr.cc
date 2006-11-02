
#include <rr.h>
#include <dg.h>
#include <dg.templ.h>
#include <strategy.h>
#include <code.h>
#include <graph_registry.h>
#include <extract.h>

using namespace libint2;

RecurrenceRelation::RecurrenceRelation()
{
}

RecurrenceRelation::~RecurrenceRelation()
{
}

//
// Generate code for this recurrence relation:
// 1) append target and children to a DirectedGraph dg
// 2) set their code symbols
// 3) apply IntSet_to_Ints
// 4) Apply RRs such that no additional vertices appear
// 5) call dg->generate_code()
//
void
RecurrenceRelation::generate_code(const SafePtr<CodeContext>& context,
                                  const SafePtr<ImplicitDimensions>& dims,
				  const std::string& funcname,
                                  std::ostream& decl, std::ostream& def)
{
  SafePtr<DirectedGraph> dg = generate_graph_();
  
  // Intermediates in RR code are either are automatic variables or have to go on vstack
  dg->registry()->stack_name("inteval->vstack");
  // No need to return the targets via inteval's targets
  dg->registry()->return_targets(false);
  
  // Assign symbols for the target and source integral sets
  SafePtr<CodeSymbols> symbols(new CodeSymbols);
  assign_symbols_(symbols);
  // Traverse the graph
  dg->optimize_rr_out();
  dg->traverse();
#if DEBUG
  dg->debug_print_traversal(std::cout);
  cout << "The number of vertices = " << dg->num_vertices() << endl;
#endif
  // Generate code
  SafePtr<MemoryManager> memman(new WorstFitMemoryManager());
  SafePtr<ImplicitDimensions> localdims = adapt_dims_(dims);
  dg->generate_code(context,memman,localdims,symbols,funcname,decl,def);
  
  // extract all external symbols -- these will be members of the evaluator structure
  SafePtr<ExtractExternSymbols> extractor(new ExtractExternSymbols);
  dg->foreach(*extractor);
  const ExtractExternSymbols::Symbols& externsymbols = extractor->symbols();

#if 0
  // print out the symbols
  std::cout << "Recovered symbols from DirectedGraph for " << label() << std::endl;
  typedef ExtractExternSymbols::Symbols::const_iterator citer;
  citer end = externsymbols.end();
  for(citer t=externsymbols.begin(); t!=end; ++t)
    std::cout << *t << std::endl;
#endif

  // get this RR InstanceID
  RRStack::InstanceID myid = RRStack::Instance()->find(EnableSafePtrFromThis<this_type>::SafePtr_from_this()).first;

  // For each task which requires this RR:
  // 1) update max stack size
  // 2) append external symbols from this RR to its list
  LibraryTaskManager& taskmgr = LibraryTaskManager::Instance();
  typedef LibraryTaskManager::TasksCIter tciter;
  const tciter tend = taskmgr.last();
  for(tciter t=taskmgr.first(); t!=tend; ++t) {
    const SafePtr<TaskExternSymbols> tsymbols = t->symbols();
    if (tsymbols->find(myid)) {
      // update max stack size
      t->params()->max_vector_stack_size(memman->max_memory_used());
      // add external symbols
      tsymbols->add(externsymbols);
    }
  }
  

    
  dg->reset();
}

SafePtr<DirectedGraph>
RecurrenceRelation::generate_graph_()
{
  SafePtr<DirectedGraph> dg(new DirectedGraph);
  dg->append_target(rr_target());
  for(int c=0; c<num_children(); c++)
    dg->append_vertex(rr_child(c));
#if DEBUG
  cout << "RecurrenceRelation::generate_code -- the number of integral sets = " << dg->num_vertices() << endl;
#endif
  // Always need to unroll integral sets
  SafePtr<Strategy> strat(new Strategy(1000000000));
  SafePtr<Tactic> ntactic(new NullTactic);
  dg->apply(strat,ntactic);
#if DEBUG
  cout << "RecurrenceRelation::generate_code -- the number of integral sets + integrals = " << dg->num_vertices() << endl;
#endif
  // Mark children sets and their descendants to not compute
  for(int c=0; c<num_children(); c++)
    dg->apply_at<&DGVertex::not_need_to_compute>(rr_child(c));
  // Apply recurrence relations using existing vertices on the graph (i.e.
  // such that no new vertices appear)
  SafePtr<Tactic> ztactic(new ZeroNewVerticesTactic(dg));
  dg->apply(strat,ztactic);
#if DEBUG
  cout << "RecurrenceRelation::generate_code -- should be same as previous = " << dg->num_vertices() << endl;
#endif
  
  return dg;
}

void
RecurrenceRelation::assign_symbols_(SafePtr<CodeSymbols>& symbols)
{
  // Set symbols on the target and children sets
  rr_target()->set_symbol("target");
  symbols->append_symbol("target");
  for(int c=0; c<num_children(); c++) {
    ostringstream oss;
    oss << "src" << c;
    string symb = oss.str();
    rr_child(c)->set_symbol(symb);
    symbols->append_symbol(symb);
  }
}

SafePtr<ImplicitDimensions>
RecurrenceRelation::adapt_dims_(const SafePtr<ImplicitDimensions>& dims) const
{
  return dims;
}

const std::string&
RecurrenceRelation::description() const
{
  return label();
}

///////////////

SafePtr<RRStack>
RRStack::rrstack_;

SafePtr<RRStack>&
RRStack::Instance() {
  if (!rrstack_) {
    SafePtr<RRStack> tmpstack(new RRStack);
    rrstack_ = tmpstack;
  }
  return rrstack_;
}

void
RRStack::add(const SafePtr<RRStack>& rrs)
{
  for(citer_type it=rrs->begin(); it != rrs->end(); it++) {
    find((*it).second.second);
  }
}

void
RRStack::remove(const data_type& rr)
{
  parent_type::remove(rr);
}

