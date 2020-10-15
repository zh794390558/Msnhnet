#define USE_ARM 1
#ifdef USE_ARM
#include "Msnhnet/layers/arm/MsnhConvolutionDepthwise3x3s2.h"
namespace Msnhnet
{
    void ConvolutionalDepthwiseLayerArm3x3s2::convdepthwise3x3s2Neon(float *const &src, const int &inWidth, const int &inHeight,  const int &inChannel, float *const &kernel,
                                 float* &dest, const int &outWidth, const int &outHeight, const int &outChannel){
        int ccOutChannel = outChannel >> 1;
        int ccRemainOutChannel = ccOutChannel << 1;

        const int in_size = inWidth * inHeight;
        const int out_size = outWidth * outHeight;
        const int group = inChannel;
#if USE_OMP
    #pragma omp parallel for num_threads(OMP_THREAD)
#endif 
        for(int g = 0; g < group; g++){
            float *dest0 = dest + g * out_size;
            const float* kernel0 = kernel + g * 9;

            float* destptr0 = dest0;

            const float* src0 = src + g * in_size;

            const float* r0 = src0;
            const float* r1 = src0 + inWidth;
            const float* r2 = src0 + inWidth * 2;
            const float* r3 = src0 + inWidth * 3;

#if USE_NEON
            float32x4_t k012 = vld1q_f32(k0);
            float32x4_t k345 = vld1q_f32(k0 + 3);
            float32x4_t k678 = vld1q_f32(k0 + 6);
            k012 = vsetq_lane_f32(0.f, k012, 3);
            k345 = vsetq_lane_f32(0.f, k345, 3);
            k678 = vsetq_lane_f32(0.f, k678, 3);
#else
            const float* k0 = kernel0;
            const float* k1 = kernel0 + 3;
            const float* k2 = kernel0 + 6;
#endif 

            int i = 0;
            
            for(; i < outHeight; i++){
#if USE_NEON
                int nn = outWidth >> 2;
                int remain = outWidth & 3;
#else
                int remain = outWidth;
#endif

#if USE_NEON

#if __aarch64__
                throw Exception(1, "Error: armv8 temporarily not supported!", __FILE__, __LINE__, __FUNCTION__);
#else  
                if(nn > 0){
                    asm volatile(
                         "0:                             \n"
                        // 每一行连续计算4个输出，所以加载
                        // r0
                        // q8 = [a, b, c, d]
                        // q9 = [e, f, g, h]
                        "pld        [%3, #256]          \n"
                        "vld2.f32   {d16-d19}, [%3]!    \n" 

                        //sum0 = q6
                        "pld        [%1, #128]          \n"
                        "vld1.f32   {d12-d13}, [%1]     \n"


                        // q8和k012的第一个元素相乘
                        "vmul.f32   q12, q8, %e10[0]    \n"
                        // q10 = [a, b, c, d]
                        "pld        [%3, #128]          \n"
                        "vld2.f32   {d20-d21}, [%3]     \n" 

                        // q9 = [b, d, f, h] 和 k012的第二个元素相乘
                        "vmla.f32   q6, q9, %e10[1]     \n"

                        // q8 = [a, c, e, g]
                        // q10 = [i, j, k, l]
                        // q11 = [c, e, g, i]
                        "vext.32    q11, q8, q10, #1    \n"

                        // q11 = [c, e, g, i] 和 k012的第三个元素相乘
                        "vmla.f32   q12, q11, %f10[0]   \n"


                        // r1
                        "pld        [%4, #256]          \n"
                        "vld2.f32   {d16-d19}, [%4]!    \n"

                        "pld        [%4, #128]          \n"
                        "vld2.f32   {d20-d21}, [%4]     \n"

                        "vmla.f32   q6, q8, %e11[0]     \n"

                        "vext.32    q11, q8, q10, #1    \n"

                        "vmla.f32   q6, q9, %e11[1]    \n"

                        "vmla.f32   q12, q11, %f11[0]    \n"

                        //r2
                        "pld        [%5, #256]          \n"
                        "vld2.f32   {d16-d19}, [%5]!    \n"

                        "pld        [%5, #128]          \n"
                        "vld2.f32   {d20-d21}, [%5]     \n"

                        "vmla.f32   q12, q8, %e12[0]    \n"

                        "vext.32    q11, q8, q10, #1    \n"

                        "vmla.f32   q6, q9, %e12[1]     \n"

                        "vmla.f32   q12, q11, %f12[0]   \n"

                        //sum
                        "vadd.f32   q6, q6, q12         \n"

                        "vst1.f32   {d12-d13}, [%1]!    \n"

                        "subs       %0, #1              \n"
                        "bne        0b                  \n"



                        // OutputOperands 
                        : "=r"(nn),     // %0
                        "=r"(destptr0), // %1
                        "=r"(r0),     // %2
                        "=r"(r1),     // %3
                        "=r"(r2)      // %4
                        // InputOperands
                        : "0"(nn),
                        "1"(destptr0),
                        "2"(r0),
                        "3"(r1),
                        "4"(r2),
                        "w"(k012), // %10
                        "w"(k345), // %11
                        "w"(k678)  // %12
                        // Clobbers
                        : "cc", "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
                    );
                }
#endif     
#endif
                for(; remain > 0; remain--){
#if USE_NEON
                    float32x4_t r00 = vld1q_f32(r0);
                    float32x4_t r10 = vld1q_f32(r1);
                    float32x4_t r20 = vld1q_f32(r2);

                    //sum1
                    float32x4_t sum1 = vmulq_f32(r00, k012);
                    sum1 = vmlaq_f32(sum1, r10, k345);
                    sum1 = vmlaq_f32(sum1, r20, k678);

                    sum1 = vsetq_lane_f32(0.f, sum1, 3);

#if __aarch64__
                    *destptr0 = vaddvq_f32(sum1);
#else
                    float32x2_t a = vadd_f32(vget_low_f32(sum1), vget_high_f32(sum1));
                    a = vpadd_f32(a, a);
                    *destptr0 = vget_lane_f32(a, 0);
#endif
                   
#else
                    float sum1 = 0.f;
                    sum1 += r0[0] * k0[0];
                    sum1 += r0[1] * k0[1];
                    sum1 += r0[2] * k0[2];
                    sum1 += r1[0] * k1[0];
                    sum1 += r1[1] * k1[1];
                    sum1 += r1[2] * k1[2];
                    sum1 += r2[0] * k2[0];
                    sum1 += r2[1] * k2[1];
                    sum1 += r2[2] * k2[2];

                    *destptr0 = sum1;
#endif
                    r0 += 2;
                    r1 += 2;
                    r2 += 2;
                    destptr0++;
                }
                r0 += 2 * (inWidth - outWidth);
                r1 += 2 * (inWidth - outWidth);
                r2 += 2 * (inWidth - outWidth);
            }
        }
    }
}

#endif