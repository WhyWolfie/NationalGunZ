/////////////////////////////////////////////////////////////
// cxr_test.h
//
// This file was generated by CXR, the literal string encryptor.
// CXR, Copyright 2002, Smaller Animals Software, Inc., all rights reserved.
//
// Please do not edit this file. Any changes here will be overwritten on the next compile.
// If you wish to make changes to a string, please edit:
//     test.cxr
//

/////////////////////////////////////////////////////////////

#pragma once
#include "cxr_inc.h"

// cxr 적용 테스트입니당

///////////////////////////
#ifdef _USING_CXR
#define STR_CXRTEST1 	_CXR("\x88\x8b\x83\x8e\x8e\x8f\x8a\x80\x8f\x8f\x88\x82\x8e\x82\x8f\x8d\x8f\x8f\x81\x87\x81\x8a\x81\x8a\x8e\x8a\x80\x86\x82\x80\x8e\x8d\x83\x8f\x89\x8e\x86\x85\x88\x81\x89\x83\x8a\x84\x86\x8f\x83\x8d\x81\x8c\x8d\x8b\x89\x82\x8f\x8f\x8e\x8f\x8a\x8b\x89\x87\x8e\x8f\x82\x80")
#else
#define STR_CXRTEST1 _CXR("Your all bases are belong to us.")
#endif

///////////////////////////
#ifdef _USING_CXR
#define STR_CXRTEST2 	_CXR("\x84\x83\x83\x81\x8d\x8e\x85\x82\x8b\x86\x8d\x81\x84\x86\x8f\x80\x8e\x88\x8a\x85")// 디파인 뒤에 주석이 달린 경우를 테스트
#else
#define STR_CXRTEST2 _CXR("It's You!")// 디파인 뒤에 주석이 달린 경우를 테스트
#endif

///////////////////////////
#ifdef _USING_CXR
#define STR_CXRTEST3 	_CXR("\x89\x8f\x85\x85\x8e\x8e\x88\x81\x88\x82")		// 디파인 뒤에 공백을 두고 주석이 달린 경우를 테스트
#else
#define STR_CXRTEST3 _CXR("Aye.")		// 디파인 뒤에 공백을 두고 주석이 달린 경우를 테스트
#endif


