
#include <typelist.h>

#ifndef _libint2_src_bin_libint_integral_h_
#define _libint2_src_bin_libint_integral_h_

namespace libint2 {

  /**
     O is an operator
     TList is a typelist TList1 that consists 2 second-level typelists: TList2 for bra and TList3 for ket
     TList2 and TList3 consist of O::np types which specify basis functions for each particle in bra/ket
  */
  
  // Tags for first and second typelists
  typedef Int2Type<1> Tag1;
  typedef Int2Type<2> Tag2;
  typedef Int2Type<2> Tag3;

  template <class O, class BraList, class KetList>
    class GenIntegral
    {
    public:
      GenIntegral(BraList bra, KetList ket) :
        bra_(bra), ket_(ket) {}
      virtual ~GenIntegral() {}

    private:
      static O O_;
      BraList bra_;
      KetList ket_;

    };


  /**
     This is an abstract base for sets of all types of integrals. Functions can be of any type
     derived from BasisFunctionSet.
  */
  template <class BasisFunctionSet> class IntegralSetIter;
  template <class BasisFunctionSet> class IntegralSet {

  public:
    virtual ~IntegralSet() {};

    /// Equivalence operator
    virtual bool equiv(const IntegralSet*) const =0;

    /// Obtain pointers to ith BasisFunctionSet for particle p in bra
    virtual const BasisFunctionSet* bra(unsigned int p, unsigned int i) const =0;
    /// Obtain pointers to ith BasisFunctionSet for particle p in ket
    virtual const BasisFunctionSet* ket(unsigned int p, unsigned int i) const =0;

  };

  /**
     GenIntegralSet uses functions derived from BFS
  */
  template <class Oper, class BFS, class BraSetType, class KetSetType> class GenIntegralSet :
    public IntegralSet<BFS>
    {
    public:
      /** No constructors are public since this is a singleton-like quantity.
          Instead, access is provided through derived class's Instance().
      */
      virtual ~GenIntegralSet();
      
      /// Equivalence operator
      virtual bool equiv(const IntegralSet<BFS>*) const;
        
      /// Obtain BFsets members
      const BFS* bra(unsigned int p, unsigned int i) const;
      const BFS* ket(unsigned int p, unsigned int i) const;

      typedef BraSetType BraType;
      typedef KetSetType KetType;
      typedef Oper OperatorType;

      /// Obtain a copy of bra
      BraType* bra() const;
      /// Obtain a copy of ket
      KetType* ket() const;

    protected:
      // Basic Integral constructor
      GenIntegralSet(const BraSetType& bra, const KetSetType& ket);
      
      BraSetType bra_;
      KetSetType ket_;

    private:
      //
      // All integrals are Singletons by nature, therefore they must be treated as such
      // 1) No public constructors are provided
      // 2) protected members are provided to implement Singleton-type functionality
      //
      GenIntegralSet();
      GenIntegralSet(const GenIntegralSet&);
      // Copy is not permitted
      GenIntegralSet& operator=(const GenIntegralSet& source);

    };


  template <class Oper, class BFS, class BraSetType, class KetSetType>
    GenIntegralSet<Oper,BFS,BraSetType,KetSetType>::GenIntegralSet(const BraSetType& bra, const KetSetType& ket) :
    bra_(bra), ket_(ket)
    {
      if (Oper::Properties::np != bra.num_part())
        throw std::runtime_error("GenIntegralSet<Oper,BFS,BraSetType,KetSetType>::GenIntegralSet(bra,ket) -- number of particles in bra doesn't match that in the operator");
      if (Oper::Properties::np != ket.num_part())
        throw std::runtime_error("GenIntegralSet<Oper,BFS,BraSetType,KetSetType>::GenIntegralSet(bra,ket) -- number of particles in ket doesn't match that in the operator");
    }

  template <class Oper, class BFS, class BraSetType, class KetSetType>
    GenIntegralSet<Oper,BFS,BraSetType,KetSetType>::~GenIntegralSet()
    {
    }

  template <class Oper, class BFS, class BraSetType, class KetSetType>
    const BFS*
    GenIntegralSet<Oper,BFS,BraSetType,KetSetType>::bra(unsigned int p, unsigned int i) const
    {
      return bra_.member(p,i);
    }
    
  template <class Oper, class BFS, class BraSetType, class KetSetType>
    const BFS*
    GenIntegralSet<Oper,BFS,BraSetType,KetSetType>::ket(unsigned int p, unsigned int i) const
    {
      return ket_.member(p,i);
    }
    
  template <class Oper, class BFS, class BraSetType, class KetSetType>
    typename GenIntegralSet<Oper,BFS,BraSetType,KetSetType>::BraType*
    GenIntegralSet<Oper,BFS,BraSetType,KetSetType>::bra() const
    {
      return new BraSetType(bra_);
    }
    
  template <class Oper, class BFS, class BraSetType, class KetSetType>
    typename GenIntegralSet<Oper,BFS,BraSetType,KetSetType>::KetType*
    GenIntegralSet<Oper,BFS,BraSetType,KetSetType>::ket() const
    {
      return new KetSetType(ket_);
    }
    
  template <class Oper, class BFS, class BraSetType, class KetSetType>
    bool
    GenIntegralSet<Oper,BFS,BraSetType,KetSetType>::equiv(const IntegralSet<BFS>* a) const
    {
      const GenIntegralSet<Oper,BFS,BraSetType,KetSetType>* a_cast = static_cast< const GenIntegralSet<Oper,BFS,BraSetType,KetSetType>* >(a);
      if (a_cast == 0)
        assert(false);
      bool bra_equiv = bra_.equiv(a_cast->bra_);
      bool ket_equiv = ket_.equiv(a_cast->ket_);
      return bra_equiv && ket_equiv;
    }

  /** VectorBraket is a std::vector-based type that can be used as a BraSetType or a KetSetType parameter
      to construct an instance of GenIntegralSet
  */
  template <class BFS> class VectorBraket {

  public:
    typedef vector<BFS> BFSVector;
    typedef vector< BFSVector > BFSMatrix;

    VectorBraket(const BFSMatrix&);
    VectorBraket(const VectorBraket&);
    ~VectorBraket() throw();

    /// Comparison function
    bool equiv(const VectorBraket&) const;
    /// Returns pointer to the i-th function for particle p
    const BFS* member(unsigned int p, unsigned int i) const;
    /// Sets i-th function for particle p
    void set_member(const BFS&, unsigned int p, unsigned int i);
    /// Returns the number of BFS for particle p
    const unsigned int num_members(unsigned int p) const;
    /// Returns the number of particles
    const unsigned int num_part() const;

  private:
    
    BFSMatrix bfs_;

  };

  template <class BFS>
    VectorBraket<BFS>::VectorBraket(const BFSMatrix& bfs) :
    bfs_(bfs)
    {
    }

  template <class BFS>
    VectorBraket<BFS>::VectorBraket(const VectorBraket& a) :
    bfs_(a.bfs_)
    {
    }

  template <class BFS>
    VectorBraket<BFS>::~VectorBraket() throw()
    {
    }

  template <class BFS>
    const BFS*
    VectorBraket<BFS>::member(unsigned int p, unsigned int i) const
    {
      return &bfs_.at(p).at(i);
    }

  template <class BFS>
    void
    VectorBraket<BFS>::set_member(const BFS& bfs, unsigned int p, unsigned int i)
    {
      bfs_.at(p).at(i) = bfs;
    }

  template <class BFS>
    const unsigned int
    VectorBraket<BFS>::num_members(unsigned int p) const
    {
      return bfs_.at(p).size();
    }

  template <class BFS>
    const unsigned int
    VectorBraket<BFS>::num_part() const
    {
      return bfs_.size();
    }

  template <class BFS>
    bool
    VectorBraket<BFS>::equiv(const VectorBraket<BFS>& a) const
    {
      return bfs_ == a.bfs_;
    }

  /**
     Most basic type -- TwoPRep_11_11 --
     has one bfs for each particle in bra and ket.
     Note that GenIntegralSet is initialized with an abstract type libint2::BFSet,
     from which BFS derives.
  */
  template <class BFS> class TwoPRep_11_11 :
    public GenIntegralSet<TwoERep, BFSet, VectorBraket<BFS>, VectorBraket<BFS> >,
    public DGVertex
    {
    public:
      typedef VectorBraket<BFS> BraType;
      typedef VectorBraket<BFS> KetType;

      /* This "constructor" takes basis function sets, in Mulliken ordering.
         Returns a pointer to a unique instance, a la Singleton
      */
      static TwoPRep_11_11* Instance(const BFS& bra0, const BFS& ket0, const BFS& bra1, const BFS& ket1, unsigned int m);
      /// Returns a pointer to a unique instance, a la Singleton
      static TwoPRep_11_11* Instance(const VectorBraket<BFS>& bra, const VectorBraket<BFS>& ket, unsigned int m);
      
      unsigned int m() const { return m_; };
      
      /// Specialization of DGVertex's equiv
      bool equiv(const DGVertex*) const;

      void print(std::ostream& os = std::cout) const;
      

    private:
      unsigned int m_;  // auxiliary index

      // Default and copy constructors are not allowed
      TwoPRep_11_11();
      TwoPRep_11_11(const TwoPRep_11_11&);      
      // This constructor is also private and not implemented since all Integral's are Singletons. Use Instance instead.
      TwoPRep_11_11(const VectorBraket<BFS>& bra, const VectorBraket<BFS>& ket, unsigned int m);

      /// stack_ of pointers to objects used to check whether an object already exists
      static vector< TwoPRep_11_11* > stack_;

    };

  template <class BFS>
    vector< TwoPRep_11_11<BFS>* > TwoPRep_11_11<BFS>::stack_(0);

  template <class BFS>
    TwoPRep_11_11<BFS>::TwoPRep_11_11(const VectorBraket<BFS>& bra, const VectorBraket<BFS>& ket, unsigned int m) :
    GenIntegralSet<TwoERep, BFSet, BraType, KetType>(bra, ket), DGVertex(), m_(m)
    {
      if (bra.num_members(0) != 1)
        throw std::runtime_error("TwoPRep_11_11<BFS>::TwoPRep_11_11(bra,ket) -- number of BFSs in bra for particle 0 must be 1");
      if (bra.num_members(1) != 1)
        throw std::runtime_error("TwoPRep_11_11<BFS>::TwoPRep_11_11(bra,ket) -- number of BFSs in bra for particle 1 must be 1");
      if (ket.num_members(0) != 1)
        throw std::runtime_error("TwoPRep_11_11<BFS>::TwoPRep_11_11(bra,ket) -- number of BFSs in ket for particle 0 must be 1");
      if (ket.num_members(1) != 1)
        throw std::runtime_error("TwoPRep_11_11<BFS>::TwoPRep_11_11(bra,ket) -- number of BFSs in ket for particle 1 must be 1");
    }

  template <class BFS>
    TwoPRep_11_11<BFS>*
    TwoPRep_11_11<BFS>::Instance(const VectorBraket<BFS>& bra, const VectorBraket<BFS>& ket, unsigned int m)
    {
      TwoPRep_11_11* const this_int = new TwoPRep_11_11<BFS>(bra,ket,m);
      int stack_size = stack_.size();
      for(int i=0; i<stack_size; i++) {
        if (this_int->equiv(stack_[i])) {
          delete this_int;
          return stack_[i];
        }
      }
      stack_.push_back(this_int);
      return this_int;
    }

  template <class BFS>
    TwoPRep_11_11<BFS>*
    TwoPRep_11_11<BFS>::Instance(const BFS& bra0, const BFS& ket0, const BFS& bra1, const BFS& ket1, unsigned int m)
    {
      vector<BFS> vbra0;  vbra0.push_back(bra0);
      vector<BFS> vbra1;  vbra1.push_back(bra1);
      vector<BFS> vket0;  vket0.push_back(ket0);
      vector<BFS> vket1;  vket1.push_back(ket1);
      vector< vector<BFS> > vvbra;  vvbra.push_back(vbra0);  vvbra.push_back(vbra1);
      vector< vector<BFS> > vvket;  vvket.push_back(vket0);  vvket.push_back(vket1);
      VectorBraket<BFS> bra(vvbra);
      VectorBraket<BFS> ket(vvket);
      return Instance(bra,ket,m);
    }
  
  template <class BFS>
    bool
    TwoPRep_11_11<BFS>::equiv(const DGVertex* a) const
    {
      // check the type first
      const TwoPRep_11_11<BFS>* a_cast = dynamic_cast< const TwoPRep_11_11<BFS>* >(a);
      if (!a_cast)
        return false;

      bool result = GenIntegralSet<TwoERep, BFSet, BraType, KetType>::equiv(a_cast) && (m_ == a_cast->m_);
      return result;
    }

  template <class BFS>
    void
    TwoPRep_11_11<BFS>::print(std::ostream& os) const
    {
      os << "TwoPRep_11_11: (" << bra_.member(0,0)->label() << " " << ket_.member(0,0)->label()
         << " | " << bra_.member(1,0)->label() << " " << ket_.member(1,0)->label() << ")^{" << m_ <<"}" << endl;
    };



  /**
     TypelistBraket is a typelist-based type to describe a bra or a ket
     of an integral. BFS is a base type for all members of a typelist,
     and TList is the typelist itself.
     
     Unfortunately, it seems that the current standard specification does
     not allow for this to work: templates are only processed once, not
     recursively...
  */
  template <class BFS, class TList> class TypelistBraket {
  public:
    typedef TList BFSMatrix;

    TypelistBraket(const TList&);
    TypelistBraket(const TypelistBraket&);
    ~TypelistBraket() throw();

    bool equiv(const TypelistBraket&) const;
    /// Returns pointer to the i-th function for particle p
    const BFS* member(unsigned int p, unsigned int i) const;
    /// Returns the number of BFS for particle p
    const unsigned int num_members(unsigned int p) const;
    /// Returns the number of particles
    const unsigned int num_part() const;

  private:

    BFSMatrix bfs_;

  };

  /*template <class BFS, class TList>
    TypelistBraket<BFS,TList>::TypelistBraket(const TList& bfs) :
    bfs_(bfs)
  {
  }

  template <class BFS, class TList>
    TypelistBraket<BFS,TList>::TypelistBraket(const TypelistBraket<BFS,TList>& a) :
    bfs_(a.bfs_)
  {
  }

  template <class BFS, class TList>
    TypelistBraket<BFS,TList>::~TypelistBraket() throw()
  {
  }

  template <class BFS, class TList>
    const BFS*
    TypelistBraket<BFS,TList>::member(unsigned int p, unsigned int i) const
  {
    return &bfs_.at(p).at(i);
  }
  
  template <class BFS, class TList>
    const unsigned int
    TypelistBraket<BFS,TList>::num_part() const
  {
    return Length<TList>::value;
  }

  template <class BFS, class TList>
    const unsigned int
    TypelistBraket<BFS,TList>::num_members(unsigned int p) const
    {
    }
  
  /*template <class BFS, class TList>
    bool
    TypelistBraket<BFS, TList>::equiv(const TypelistBraket<BFS,TList>& a) const
  {
    return bfs_ == a.bfs_;
  }*/
  

};

#endif
