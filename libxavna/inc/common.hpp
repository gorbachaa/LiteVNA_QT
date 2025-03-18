#ifndef COMMON_HPP
#define COMMON_HPP

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/StdVector>


using namespace Eigen;
using namespace std;


namespace xaxaxa {
    using VNARawValue = Matrix2cd;
    using VNACalibratedValue = Matrix2cd;
    inline Matrix2cd mirror( Matrix2cd a )
    {
        complex<double> tmp;
        tmp = a( 0, 0 );
        a( 0, 0 ) = a( 1, 1 );
        a( 1, 1 ) = tmp;
        tmp = a( 0, 1 );
        a( 0, 1 ) = a( 1, 0 );
        a( 1 ,0 ) = tmp;
        return a;
    }
}

#endif
