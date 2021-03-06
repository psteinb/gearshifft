#ifndef BENCHMARK_HPP_
#define BENCHMARK_HPP_
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API

#include "application.hpp"
#include "benchmark_suite.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

namespace gearshifft {

  /// List alias
  template<typename... Types>
  using List = boost::mpl::list<Types...>;

  /**
   * Benchmark API class for clients.
   *
   */
  template<typename Context>
  class Benchmark {
    using AppT = Application<Context>;

  public:
    Benchmark() = default;

    ~Benchmark() {
      if(AppT::getInstance().isContextCreated()) {
        AppT::getInstance().destroyContext();
      }
    }

    void configure(int argc, char* argv[]) {
      configured_ = false;
      std::vector<char*> vargv(argv, argv+argc);
      boost_vargv_.clear();
      boost_vargv_.emplace_back(argv[0]); // [0] = name of application

      if( Context::options().parse(vargv, boost_vargv_) ) {
        if( Context::options().getListDevices() ) {
          std::cout << Context::get_device_list() << std::endl;
        }
      }
      else
        configured_ = true;
    }

    template<typename T_FFT_Is_Normalized,
             typename T_FFTs,
             typename T_Precisions>
    int run() {
      if(Context::options().getListDevices())
        return 0;
      if(configured_==false)
        return 1;

      AppT::getInstance().createContext();

      auto init_function = []() {
        BenchmarkSuite<Context, T_FFT_Is_Normalized, T_FFTs, T_Precisions> instance;
        ::boost::unit_test::framework::master_test_suite().add( instance() );
        return true;
      };

      int r = ::boost::unit_test::unit_test_main( init_function,
                                                  boost_vargv_.size(),
                                                  boost_vargv_.data() );

      AppT::getInstance().destroyContext();
      AppT::getInstance().dumpResults();
      return r;
    }

  private:
    bool configured_ = false;
    std::vector<char*> boost_vargv_;
  };

} // gearshifft

#endif // BENCHMARK_HPP_
