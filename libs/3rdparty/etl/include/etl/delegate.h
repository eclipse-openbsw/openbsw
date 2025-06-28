///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
https://www.etlcpp.com

Copyright(c) 2021 John Wellbelove

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef ETL_DELEGATE_INCLUDED
#define ETL_DELEGATE_INCLUDED

#include "platform.h"

#if ETL_USING_CPP11 && !defined(ETL_DELEGATE_FORCE_CPP03_IMPLEMENTATION)
  #include "private/delegate_cpp11.h" // IWYU pragma: export
#else
  #include "private/delegate_cpp03.h" // IWYU pragma: export
#endif

namespace etl {
    //***************************************************************************
  template<typename>
  struct closure;
  
  template<typename R, typename P0>
  struct closure<R(P0)>
  {
      using fct = delegate<R(P0)>;
  
      closure(fct const& f, P0 const p0) : _f(f), _p0(p0) {}
  
      R operator()() const { return _f(_p0); }
  
  private:
      fct _f;
      P0 _p0;
  };
  
  template<typename R, typename P0, typename P1>
  struct closure<R(P0, P1)>
  {
      using fct = delegate<R(P0, P1)>;
  
      closure(fct const& f, P0 const p0, P1 const p1) : _f(f), _p0(p0), _p1(p1) {}
  
      R operator()() const { return _f(_p0, _p1); }
  
  private:
      fct _f;
      P0 _p0;
      P1 _p1;
  };
  
  template<typename R, typename P0, typename P1, typename P2>
  struct closure<R(P0, P1, P2)>
  {
      using fct = delegate<R(P0, P1, P2)>;
  
      closure(fct const& f, P0 const p0, P1 const p1, P2 const p2) : _f(f), _p0(p0), _p1(p1), _p2(p2)
      {}
  
      R operator()() const { return _f(_p0, _p1, _p2); }
  
  private:
      fct _f;
      P0 _p0;
      P1 _p1;
      P2 _p2;
  };
  
  template<typename R, typename P0, typename P1, typename P2, typename P3>
  struct closure<R(P0, P1, P2, P3)>
  {
      using fct = delegate<R(P0, P1, P2, P3)>;
  
      closure(fct const& f, P0 const p0, P1 const p1, P2 const p2, P3 const p3)
      : _f(f), _p0(p0), _p1(p1), _p2(p2), _p3(p3)
      {}
  
      R operator()() const { return _f(_p0, _p1, _p2, _p3); }
  
  private:
      fct _f;
      P0 _p0;
      P1 _p1;
      P2 _p2;
      P3 _p3;
  };
  
  template<typename R, typename P0, typename P1, typename P2, typename P3, typename P4>
  struct closure<R(P0, P1, P2, P3, P4)>
  {
      using fct = delegate<R(P0, P1, P2, P3, P4)>;
  
      closure(fct const& f, P0 const p0, P1 const p1, P2 const p2, P3 const p3, P4 const p4)
      : _f(f), _p0(p0), _p1(p1), _p2(p2), _p3(p3), _p4(p4)
      {}
  
      R operator()() const { return _f(_p0, _p1, _p2, _p3, _p4); }
  
  private:
      fct _f;
      P0 _p0;
      P1 _p1;
      P2 _p2;
      P3 _p3;
      P4 _p4;
  };

}

#endif
