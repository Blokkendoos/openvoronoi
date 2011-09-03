/*  
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  
 *  This file is part of OpenCAMlib.
 *
 *  OpenCAMlib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenCAMlib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenCAMlib.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cassert>

#include "voronoivertex.hpp"
#include "numeric.hpp"

namespace ovd {


int VoronoiVertex::count = 0;

    VoronoiVertex::VoronoiVertex() {
        init();
        status = UNDECIDED;
        type = NORMAL;
    }
    /// construct vertex at position p with type t
    VoronoiVertex::VoronoiVertex( Point p, VoronoiVertexStatus st) {
        position=p;
        status=st;
        type = NORMAL;
        init();
    }
    
    VoronoiVertex::VoronoiVertex( Point p, VoronoiVertexStatus st, VoronoiVertexType t) {
        position=p;
        status=st;
        type=t;
        init();
    }
    void VoronoiVertex::init() {
        index = count;
        count++;
        in_queue = false;
    }
    void VoronoiVertex::reset() {
        in_queue = false;
        status = UNDECIDED;
    }
    
    // set the generators and position the vertex
    void VoronoiVertex::set_generators(const Point& pi, const Point& pj, const Point& pk) {
        set_J(pi,pj,pk);
        set_position();
    }
    
    /// based on precalculated J2, J3, J4, _pk, calculate the H determinant for input Point pl
    /// Eq.(20) from Sugihara&Iri 1994
    /// H<0 means pl is inside the circle
    /// H==0 on the edge of the circle
    /// H>9 outside the circle
    double VoronoiVertex::detH(const Point& pl) const {
        return J2*(pl.x- _pk.x) - J3*(pl.y-_pk.y) + 0.5*J4*(square(pl.x-_pk.x) + square(pl.y-_pk.y));
    }
    void VoronoiVertex::set_position() {
        double w = J4;
        double x = -J2/w + _pk.x;
        double y =  J3/w + _pk.y;
        position =  Point(x,y);
    }

    // this allows sorting points
    /*
    bool operator<(const VertexProps& other) const {
        return ( abs(this->H) < abs(other.H) );
    }*/
    /// set the J values
    /// pi, pj, pk define the three PointGenerators that position this vertex
    void VoronoiVertex::set_J(const Point& pi, const Point& pj, const Point& pkin) { 
        // 1) i-j-k should come in CCW order
        
        Point pi_,pj_,pk_;
        if ( pi.isRight(pj,pkin) ) {
            pi_ = pj;
            pj_ = pi;
            pk_ = pkin;
        } else {
            pi_ = pi;
            pj_ = pj;
            pk_ = pkin;
        }
        assert( !pi_.isRight(pj_,pk_) );
        // 2) point _pk should have the largest angle 
        // largest angle is opposite longest side.
        Point pi__,pj__,pk__;
        pi__ = pi_;                          
        pj__ = pj_;                          
        pk__ = pk_;
        double longest_side = (pi_ - pj_).norm();
        if (  (pj_ - pk_).norm() > longest_side ) {
            longest_side = (pj_ - pk_).norm(); //j-k is longest, so i should be new k
            pk__ = pi_;                         // old  i-j-k 
            pi__ = pj_;                         // new  k-i-j
            pj__ = pk_;
        }
        if ( (pi_ - pk_).norm() > longest_side ) { // i-k is longest, so j should be new k                    
            pk__ = pj_;                          // old  i-j-k
            pj__ = pi_;                          // new  j-k-i
            pi__ = pk_;
        }
        
        assert( !pi__.isRight(pj__,pk__) );
        assert( (pi__ - pj__).norm() >=  (pj__ - pk__).norm() );
        assert( (pi__ - pj__).norm() >=  (pk__ - pi__).norm() );
        // storing J2,J3,J4, and _pk allows us to call detH() later 
        _pk = pk__;
        
        J2 = detH_J2( pi__, pj__);
        J3 = detH_J3( pi__, pj__);
        J4 = detH_J4( pi__, pj__);
        assert( J4 != 0.0 ); // we need to divide by J4 later, so it better not be zero...
    }
    // calculate J2
    double VoronoiVertex::detH_J2(const Point& pi, const Point& pj) {
        return (pi.y- _pk.y)*(square(pj.x- _pk.x)+square(pj.y- _pk.y))/2 - (pj.y- _pk.y)*(square(pi.x- _pk.x)+square(pi.y- _pk.y))/2;
    }
    // calculate J3
    double VoronoiVertex::detH_J3(const Point& pi, const Point& pj) {
        return (pi.x- _pk.x)*(square(pj.x- _pk.x)+square(pj.y- _pk.y))/2 - (pj.x- _pk.x)*(square(pi.x- _pk.x)+square(pi.y- _pk.y))/2;
    }
    // calculate J4
    double VoronoiVertex::detH_J4(const Point& pi, const Point& pj) {
        return (pi.x- _pk.x)*(pj.y- _pk.y) - (pj.x- _pk.x)*(pi.y- _pk.y);
    }

} // end ocl namespace
