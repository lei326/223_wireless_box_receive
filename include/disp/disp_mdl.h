#pragma once
#include <stdint.h>

#include "xm_common.h"
#include "xm_middleware_def.h"

class DispMdl
{
public:
    static DispMdl* Instance();
    int Start();
    int Stop();
    int SendFrame(const unsigned char* data, unsigned int len, bool key_frame);
    void ResetDecoder();

private:
    DispMdl();
    ~DispMdl();
    int SysInit();    
    int VoInit();      
    int VpssInit();    
    int VdecInit();     
    int Bind();        
    int Unbind();      

    static DispMdl* instance_;

    volatile bool started_;
    volatile bool wait_i_frame_;
};