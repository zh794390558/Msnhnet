#ifndef MSNHMAXPOOLING2x2S2ARM_H
#define MSNHMAXPOOLING2x2S2ARM_H

#ifdef USE_ARM
namespace Msnhnet
{

class MsnhNet_API MaxPooling2x2s2Arm
{
public:
    //bottom: src, inWidth, inHeight, inChannel
    //top: dest, outWidth, outHeight, outChannel
    //ceil mode如果为1并且特征图长或宽为奇数，提前执行Padding一圈0即可
    void pooling(float *const &src, const int &inWidth, const int &inHeight,  const int &inChannel, 
                                    float* &dest, const int& ceilModel);
};

}
#endif
#endif