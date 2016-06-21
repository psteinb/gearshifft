#ifndef HCFFT_HPP_
#define HCFFT_HPP_

#include <array>

#include "application.hpp"
#include "timer.hpp"
#include "fft_abstract.hpp"
#include "fixture_test_suite.hpp"


#include "hcfft_helper.hpp"
#include "hcfft.h"


namespace gearshifft
{
  namespace hcfft
  {

    namespace traits
    {
      template<typename T_Precision=float>
      struct Types
      {
	using ComplexType = hcfftComplex;
	using RealType = hcfftReal;
	static constexpr hcfftType_t FFTForward = HCFFT_R2C;
	static constexpr hcfftType_t FFTComplex = HCFFT_C2C;
	static constexpr hcfftType_t FFTBackward = HCFFT_C2R;

	struct FFTExecuteForward{
	  void operator()(hcfftHandle plan, RealType* in, ComplexType* out){
	    CHECK_HCFFT(hcfftExecR2C(plan, in, out));
	  }
	  void operator()(hcfftHandle plan, ComplexType* in, ComplexType* out){
	    CHECK_HCFFT(hcfftExecC2C(plan, in, out, HCFFT_FORWARD));
	  }
	};
	struct FFTExecuteBackward{
	  void operator()(hcfftHandle plan, ComplexType* in, RealType* out){
	    CHECK_HCFFT(hcfftExecC2R(plan, in, out));
	  }
	  void operator()(hcfftHandle plan, ComplexType* in, ComplexType* out){
	    CHECK_HCFFT(hcfftExecC2C(plan, in, out, HCFFT_BACKWARD));
	  }
	};
      };

      template<>
      struct Types<double>
      {
	using ComplexType = hcfftDoubleComplex;
	using RealType = hcfftDoubleReal;
	static constexpr hcfftType FFTForward = HCFFT_D2Z;
	static constexpr hcfftType FFTComplex = HCFFT_Z2Z;
	static constexpr hcfftType FFTBackward = HCFFT_Z2D;

	struct FFTExecuteForward{
	  void operator()(hcfftHandle plan, RealType* in, ComplexType* out){
	    CHECK_HCFFT(hcfftExecD2Z(plan, in, out));
	  }
	  void operator()(hcfftHandle plan, ComplexType* in, ComplexType* out){
	    CHECK_HCFFT(hcfftExecZ2Z(plan, in, out, HCFFT_FORWARD));
	  }
	};
	struct FFTExecuteBackward{
	  void operator()(hcfftHandle plan, ComplexType* in, RealType* out){
	    CHECK_HCFFT(hcfftExecZ2D(plan, in, out));
	  }
	  void operator()(hcfftHandle plan, ComplexType* in, ComplexType* out){
	    CHECK_HCFFT(hcfftExecZ2Z(plan, in, out, HCFFT_BACKWARD));
	  }
	};
      };

    }  // namespace traits

    /**
     * HCFFT implicit context init and reset wrapper. Time is benchmarked.
     */
    struct Context {

      static const std::string title() {
	return "hcfft";
      }

      std::string getDeviceInfos() {
	auto ss = getHCCDeviceInformations(0);
	return ss;
      }

      void create() {
	//n.n.
	// CHECK_CUDA(cudaSetDevice(0));
      }

      void destroy() {
	//n.n.
	// CHECK_CUDA(cudaDeviceReset());
      }
    };


    /**
     * Estimates memory reserved by hcfft plan depending on FFT transform type
     * (HCFFT_R2C, ...) and depending on number of dimensions {1,2,3}.
     */
    template<size_t NDim>
    size_t estimateAllocSize(const std::array<unsigned,NDim>& e, hcfftHandle& plan)
    {
      
      
      return 0;
    }
    /**
     * Plan Creator depending on FFT transform type (HCFFT_R2C, ...).
     */
    template<hcfftType FFTType>
    void makePlan(hcfftHandle& plan, const std::array<unsigned,3>& e){
      hcfftHandle* plan_ptr = &plan;
      CHECK_HCFFT(hcfftPlan3d(plan_ptr, e[0], e[1], e[2], FFTType));
    }
    template<hcfftType FFTType>
    void makePlan(hcfftHandle& plan, const std::array<unsigned,1>& e){
      hcfftHandle* plan_ptr = &plan;
      CHECK_HCFFT(hcfftPlan1d(plan_ptr, e[0], FFTType));
    }
    template<hcfftType FFTType>
    void makePlan(hcfftHandle& plan, const std::array<unsigned,2>& e){
      hcfftHandle* plan_ptr = &plan;
      CHECK_HCFFT(hcfftPlan2d(plan_ptr, e[0], e[1], FFTType));
    }


    /**
     * hcfft plan and execution class.
     *
     * This class handles:
     * - {1D, 2D, 3D} x {R2C, C2R, C2C} x {inplace, outplace} x {float, double}.
     */
    template<typename TFFT, // see fft_abstract.hpp (FFT_Inplace_Real, ...)
	     typename TPrecision, // double, float
	     size_t   NDim // 1..3
	     >
    struct hcfftImpl
    {
      using Extent = std::array<unsigned,NDim>;
      using Types  = typename traits::Types<TPrecision>;
      using ComplexType = typename Types::ComplexType;
      using RealType = typename Types::RealType;
      using FFTExecuteForward  = typename Types::FFTExecuteForward;
      using FFTExecuteBackward = typename Types::FFTExecuteBackward;

      static constexpr
      bool IsInplace = TFFT::IsInplace;
      static constexpr
      bool IsComplex = TFFT::IsComplex;
      static constexpr
      bool Padding = IsInplace && IsComplex==false && NDim>1;
      static constexpr
      hcfftType FFTForward  = IsComplex ? Types::FFTComplex : Types::FFTForward;
      static constexpr
      hcfftType FFTBackward = IsComplex ? Types::FFTComplex : Types::FFTBackward;

      using RealOrComplexType  = typename std::conditional<IsComplex,ComplexType,RealType>::type;

      size_t n_;        // =[1]*..*[dim]
      size_t n_padded_; // =[1]*..*[dim-1]*([dim]/2+1)
      Extent extents_;

      hcfftHandle        plan_           = 0;
      RealOrComplexType* data_           = nullptr;
      ComplexType*       data_transform_ = nullptr; // intermediate buffer
      size_t             data_size_;
      size_t             data_transform_size_;
      std::vector<hc::accelerator> accs_  = hc::accelerator::get_all();
      int                device_to_use_ = 1;
      
      hcfftImpl(const Extent& cextents, int device = 1)
	: extents_(cextents)
      {
	n_ = std::accumulate(extents_.begin(),
			     extents_.end(),
			     1,
			     std::multiplies<unsigned>());
	if(Padding)
	  n_padded_ = n_ / extents_[NDim-1] * (extents_[NDim-1]/2 + 1);

	data_size_ = ( Padding ? 2*n_padded_ : n_ ) * sizeof(RealOrComplexType);
	data_transform_size_ = IsInplace ? 0 : n_ * sizeof(ComplexType);

	if(device!=device_to_use_ && device<accs_.size())
	  device_to_use_ = device;
      }

      /**
       * Returns allocated memory on device for FFT
       */
      size_t getAllocSize() {
	return data_size_ + data_transform_size_;
      }

      /**
       * Returns estimated allocated memory on device for FFT plan
       */
      size_t getPlanSize() {
	size_t size1 = 0;
	size_t size2 = 0;
	init_forward();
	size1 = estimateAllocSize(extents_,plan_);
	init_backward();
	size2 = estimateAllocSize(extents_,plan_);
	CHECK_HCFFT( hcfftDestroy(plan_) );
	return std::max(size1,size2);
      }

      // --- next methods are benchmarked ---

      void malloc() {

	data_ = hc::am_alloc(data_size_,accs_[device_to_use_],0);

	if(IsInplace){
	  data_transform_ = reinterpret_cast<ComplexType*>(data_);
	}else{
	  data_transform_ = hc::am_alloc(data_transform_size_,accs_[device_to_use_],0);
	}
      }

      // create FFT plan handle
      void init_forward() {
	makePlan<FFTForward>(plan_, extents_);
      }

      // recreates plan if needed
      void init_backward() {
	if(IsComplex==false){
	  CHECK_HCFFT(hcfftDestroy(plan_));
	  makePlan<FFTBackward>(plan_, extents_);
	}
      }

      void execute_forward() {
	FFTExecuteForward()(plan_, data_, data_transform_);
      }

      void execute_backward() {
	FFTExecuteBackward()(plan_, data_transform_, data_);
      }

      template<typename THostData>
      void upload(THostData* input) {
	if(Padding) // real + inplace + ndim>1
	  {
	    size_t w      = extents_[NDim-1] * sizeof(THostData);
	    size_t h      = n_ * sizeof(THostData) / w;
	    size_t pitch  = (extents_[NDim-1]/2+1) * sizeof(ComplexType);
	    CHECK_CUDA(cudaMemcpy2D(data_, pitch, input, w, w, h, cudaMemcpyHostToDevice));
	  }else{
	  CHECK_CUDA(cudaMemcpy(data_, input, data_size_, cudaMemcpyHostToDevice));
	}
      }

      template<typename THostData>
      void download(THostData* output) {
	if(Padding) // real + inplace + ndim>1
	  {
	    size_t w      = extents_[NDim-1] * sizeof(THostData);
	    size_t h      = n_ * sizeof(THostData) / w;
	    size_t pitch  = (extents_[NDim-1]/2+1) * sizeof(ComplexType);
	    CHECK_CUDA(cudaMemcpy2D(output, w, data_, pitch, w, h, cudaMemcpyDeviceToHost));
	  }else{
	  CHECK_CUDA(cudaMemcpy(output, data_, data_size_, cudaMemcpyDeviceToHost));
	}
      }

      void destroy() {
	hc::am_free(data_);
	if(IsInplace==false)
	  hc::am_free(data_transform_);
	CHECK_HCFFT( hcfftDestroy(plan_) );
      }
    };

    typedef gearshifft::FFT<gearshifft::FFT_Inplace_Real, hcfftImpl, TimerGPU> Inplace_Real;
    typedef gearshifft::FFT<gearshifft::FFT_Outplace_Real, hcfftImpl, TimerGPU> Outplace_Real;
    typedef gearshifft::FFT<gearshifft::FFT_Inplace_Complex, hcfftImpl, TimerGPU> Inplace_Complex;
    typedef gearshifft::FFT<gearshifft::FFT_Outplace_Complex, hcfftImpl, TimerGPU> Outplace_Complex;

  } // namespace hcfft
} // namespace gearshifft


#endif /* HCFFT_HPP_ */
