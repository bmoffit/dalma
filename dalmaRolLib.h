#ifndef __DALMAROLLIB_H__
#define __DALMAROLLIB_H__
/*----------------------------------------------------------------------------*
 *  Copyright (c) 2021        Southeastern Universities Research Association, *
 *                            Thomas Jefferson National Accelerator Facility  *
 *                                                                            *
 *    This software was developed under a United States Government license    *
 *    described in the NOTICE file included as part of this distribution.     *
 *                                                                            *
 *    Author:  Bryan Moffit                                                   *
 *             moffit@jlab.org                   Jefferson Lab, MS-12B3       *
 *             Phone: (757) 269-5660             12000 Jefferson Ave.         *
 *             Fax:   (757) 269-5800             Newport News, VA 23606       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *
 * Description:
 *     Header for Library to support piping standard out to daLogMsg
 *
 *----------------------------------------------------------------------------*/

#define DALMAGO dalmaRedirectEnable(1);
#define DALMASTOP dalmaRedirectDisable();

int32_t dalmaInit(int32_t echo);
int32_t dalmaClose();
void dalmaRedirectEnable(int echo);
void dalmaRedirectDisable();



#endif /* __DALMAROLLIB_H__ */
