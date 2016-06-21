#ifndef HCFFT_HELPER_HPP_
#define HCFFT_HELPER_HPP_

#include "hcfft.h"
#include <sstream>
#include <stdexcept>
#include <codecvt>
#include <locale>

#ifndef HCFFT_DISABLE_ERROR_CHECKING
#define CHECK_HCFFT(ans) gearshifft::hcfft::check_hcfft((ans), #ans, __FILE__, __LINE__)
#else
#define CHECK_HCFFT(ans) {}
#endif

namespace gearshifft {
namespace hcfft {

  inline
  void throw_error(int code,
                   const char* error_string,
                   const char* msg,
                   const char* func,
                   const char* file,
                   int line) {
    // cudaDeviceReset();
    throw std::runtime_error("HCFFT error "
                             +std::string(msg)
                             +" "+std::string(error_string)
                             +" ["+std::to_string(code)+"]"
                             +" "+std::string(file)
                             +":"+std::to_string(line)
                             +" "+std::string(func)
      );
  }

  

  static const char* hcfftResultToString(hcfftResult_t error) {
    switch (error) {
    case HCFFT_SUCCESS			: return "HCFFT_SUCCESS                    ";
    case HCFFT_INVALID_PLAN   		: return "HCFFT_INVALID_PLAN               ";
    case HCFFT_ALLOC_FAILED   		: return "HCFFT_ALLOC_FAILED               ";
    case HCFFT_INVALID_TYPE   		: return "HCFFT_INVALID_TYPE               ";
    case HCFFT_INVALID_VALUE  		: return "HCFFT_INVALID_VALUE              ";
    case HCFFT_INTERNAL_ERROR 		: return "HCFFT_INTERNAL_ERROR             ";
    case HCFFT_EXEC_FAILED    		: return "HCFFT_EXEC_FAILED                ";
    case HCFFT_SETUP_FAILED   		: return "HCFFT_SETUP_FAILED               ";
    case HCFFT_INVALID_SIZE   		: return "HCFFT_INVALID_SIZE               ";
    case HCFFT_UNALIGNED_DATA 		: return "HCFFT_UNALIGNED_DATA             ";
    case HCFFT_INCOMPLETE_PARAMETER_LIST 	: return "HCFFT_INCOMPLETE_PARAMETER_LIST  ";
    case HCFFT_INVALID_DEVICE 		: return "HCFFT_INVALID_DEVICE             ";
    case HCFFT_PARSE_ERROR    		: return "HCFFT_PARSE_ERROR                ";
    case HCFFT_NO_WORKSPACE		: return "HCFFT_NO_WORKSPACE               ";
    default:
      return "<unknown>";
    }
  }

  inline
  void check_hcfft(hcfftResult_t code, const char* msg, const char *func, const char *file, int line) {
    if (code != HCFFT_SUCCESS) {
      throw_error(static_cast<int>(code),
                  hcfftResultToString(code), msg, func, file, line);
    }
  }
  
  inline
  void check_hcfft(hcfftResult code, const char *func, const char *file, int line) {
    if(code) {
      throw_error(static_cast<int>(code),
                  hcfftResultToString(code), "hcfft", func, file, line);
    }
  }

  inline
  std::string getHCCDeviceInformations(int dev) {

    std::vector<hc::accelerator> accs = hc::accelerator::get_all();

    std::string value = "";

    if(dev >= accs.size()){
      std::cerr << "hcc device " << dev << " unknown\n";
      return value;
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::ostringstream info;
    int runtimeVersion = 0;
    size_t f=0, t=0;
    auto current = accs[dev];
    info << '"' << converter.to_bytes(current.get_description()) << '"'
         << ", \"Version\", " << current.get_version()
         << ", \"Memory [MiB]\", "<< current.get_dedicated_memory()
      ;

    value = info.str();
    
    return value;
  }
} // hcfft
} // gearshifft
#endif
