/*****************************************************************************
*  PDTools Protocol File                                                     *
*  Copyright (C) 2018-2019 Melux www.melux.com                               *                                                                          *                                         *
*                                                                            *
*  @file     pd_Protocol.h                                                   *
*  @brief    PDTools 网络交互协议（TCP，UDP）                                   *
*  Details.  usefor pdTools and pdTools-Server                               *
*                                                                            *
*  @author   Wanglj                                                          *
*  @version  2.0.0.0                                                         *
*  @date     2018-05-10                                                      *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark         : Description                                              *
*----------------------------------------------------------------------------*
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2018-05-10 | 1.0.0.0   | Wanglj         | Create file                     *
*----------------------------------------------------------------------------*
*  2018-12-27 | 2.0.0.0   | ChengHang      | Modify Protocol                 *
*----------------------------------------------------------------------------*
*                                                                            *
*****************************************************************************/
#ifndef _PD_PROTOCOL_H
#define _PD_PROTOCOL_H

#define TIMEOUT 5 //s
#define START '$'

#define TESTMODE 0;

/* 报文命令枚举 */
enum COMMAND{
    //new PDTools 2.x
    INIT = 'I', //初始化
    RESPONE = 'R', //响应
    IMAGES_ENABLED = 'E', //启用图像调试
    IMAGES_DISABLED = 'D', //禁用图像调试
    IMAGES_STREAM = 'T', //图像流
    IMAGES_CAPTURE = 'C', //捕获图像
    UIJSON = 'U', //UIJSON构造树
    VALUE = 'V', //请求修改
    SCAN = 'S', //扫描设备
    SYNC = 'Y', //同步设备
    FIRMWARE_START = 'F',//固件开始上传
    FIRMWARE_END = 'M',//固件上传结束
};

/* 报文图像类型枚举 */
enum IMAGETYPE{
    PALM = 'P', //掌脉
    FACE = 'F' //面容
};

/* 报文结构体 */
#define LEN_HEAD_PKG 7	// Byte = start+cmd+len+check
typedef struct pd_Package{
    unsigned char start; //起始字节 START
    enum COMMAND cmd; //命令
    unsigned char len[4]; //body长度 len = len[0]<<24 + len[1]<<16 + len[2]<<8 +len[3];
    void* body; //body
    unsigned char check; //校验码 CRC8
}pd_Package_t;

/* 图像结构体 */
#define LEN_HEAD_IMG 5
typedef struct pd_Img{
    unsigned char width[2];	// 图像宽度 width = width[0]<<8 + width[1];
    unsigned char high[2];	// 图像高度 high = high[0]<<8 + high[1];
    enum IMAGETYPE type; //图像类别
    unsigned char* Image; //图像数据
}pd_Img_t;


#endif
