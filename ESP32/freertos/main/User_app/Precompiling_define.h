#ifndef _Precompiling__H_
#define _Precompiling__H_

/*  
    this is Precompiling_define.h  feil    
    最底层的存在，几乎根 [Items.h]文件同级
*/

#define ESP32_Cavend        314
#define EY1001              001

/*      ---     */
#define Board_Name      ESP32_Cavend      //

#if (Board_Name == ESP32_Cavend)
    #define soft_versions       "EC.0.0.01"
#elif (Board_Name == EY1001)
    #define soft_versions       "EY.0.0.01"
#endif // soft_versions





#endif // !_Precompiling__H_
