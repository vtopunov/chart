#include <array>

#include <QApplication>
#include <QDir>

#include <3rdparty/gcem/include/gcem.hpp>

#include <core/math_constants.h>
#include <core/color.h>

#include "chart.h"


#pragma warning(push)
#pragma warning(disable : 26485) 


namespace
{
    using real_fn_t = double (*) (double);

    constexpr double sinc(double x) noexcept
    {
        return ( gcem::abs(x) > std::numeric_limits<double>::epsilon() ) ? ( gcem::sin(x) / x ) : ( 1.0 );
    }

    constexpr span<point_t> fn_generate( span<point_t> result, real_fn_t fn )
    {
        D_ASSERT( fn );

        const size_t size = result.size();
        const auto abscissa_max = 5 * pi_v<real_t>;
        const num_range<real_t> abscissa_range{ -abscissa_max, abscissa_max };
        const num_range<size_t> index_range{ 0_z, size - 1_z };
        const auto abscissa = lerp( index_range, abscissa_range );

        for ( size_t i = index_range.front(); i <= index_range.back(); ++i )
        {
            const auto x = abscissa( i );
            result[i] = point_t{ x, fn( x ) };
        }

        return result;
    }

    constexpr auto constexprSinc95Percent = [] ()
    {
        std::array<point_t, 300_z> result{};
        fn_generate( result, [] ( double x ) { return 0.95 * sinc( x ); } );
        return result;
    }();

    constexpr auto constexprInvsinc95Percent = [] ()
    {
        std::array<point_t, 300_z> result{};
        fn_generate( result, [] ( double x ) { return -0.95 * sinc( x ); } );
        return result;
    }();
}


int main(int argc, char* argv[])
{
    QApplication::addLibraryPath(QDir::currentPath());
    QApplication a(argc, argv);
    
    Chart chart;

    chart.margins = { 30, 30, 30, 30 };

    {
        auto sinc_vector_gen = [] ()
        {
            std::vector<point_t> result( 6000_z );
            fn_generate( result, sinc );
            return result;
        };

        auto invsinc_vector_gen = [] ()
        {
            std::vector<point_t> result( 6000_z );
            fn_generate( result, [] ( double x ) { return -sinc( x ); } );
            return result;
        };

        chart.figures.add( constexprSinc95Percent, colors::blue.with_opacity( 0.5_ur ) );
        chart.figures.add( sinc_vector_gen() );
        chart.figures.add( constexprInvsinc95Percent, colors::red.with_opacity( 0.5_ur ) );
        chart.figures.add( invsinc_vector_gen(), colors::red );
    }

    chart.resize(1280, 720);
    chart.show();

    return a.exec();
}

#pragma warning(pop)
