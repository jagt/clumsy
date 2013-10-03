/** \file
 * \brief Keyboard Keys definitions.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPKEY_H 
#define __IUPKEY_H

/* from 32 to 126, all character sets are equal,
   the key code is the same as the character code. */

#define K_SP          ' '   /* 32 (0x20) */
#define K_exclam      '!'   /* 33 */
#define K_quotedbl    '\"'  /* 34 */
#define K_numbersign  '#'   /* 35 */
#define K_dollar      '$'   /* 36 */
#define K_percent     '%'   /* 37 */
#define K_ampersand   '&'   /* 38 */
#define K_apostrophe  '\''  /* 39 */
#define K_parentleft  '('   /* 40 */
#define K_parentright ')'   /* 41 */
#define K_asterisk    '*'   /* 42 */
#define K_plus        '+'   /* 43 */
#define K_comma       ','   /* 44 */
#define K_minus       '-'   /* 45 */
#define K_period      '.'   /* 46 */
#define K_slash       '/'   /* 47 */
#define K_0           '0'   /* 48 (0x30) */
#define K_1           '1'   /* 49 */
#define K_2           '2'   /* 50 */
#define K_3           '3'   /* 51 */
#define K_4           '4'   /* 52 */
#define K_5           '5'   /* 53 */
#define K_6           '6'   /* 54 */
#define K_7           '7'   /* 55 */
#define K_8           '8'   /* 56 */
#define K_9           '9'   /* 57 */
#define K_colon       ':'   /* 58 */
#define K_semicolon   ';'   /* 59 */
#define K_less        '<'   /* 60 */
#define K_equal       '='   /* 61 */
#define K_greater     '>'   /* 62 */
#define K_question    '?'   /* 63 */
#define K_at          '@'   /* 64 */
#define K_A           'A'   /* 65 (0x41) */
#define K_B           'B'   /* 66 */
#define K_C           'C'   /* 67 */
#define K_D           'D'   /* 68 */
#define K_E           'E'   /* 69 */
#define K_F           'F'   /* 70 */
#define K_G           'G'   /* 71 */
#define K_H           'H'   /* 72 */
#define K_I           'I'   /* 73 */
#define K_J           'J'   /* 74 */
#define K_K           'K'   /* 75 */
#define K_L           'L'   /* 76 */
#define K_M           'M'   /* 77 */
#define K_N           'N'   /* 78 */
#define K_O           'O'   /* 79 */
#define K_P           'P'   /* 80 */
#define K_Q           'Q'   /* 81 */
#define K_R           'R'   /* 82 */
#define K_S           'S'   /* 83 */
#define K_T           'T'   /* 84 */
#define K_U           'U'   /* 85 */
#define K_V           'V'   /* 86 */
#define K_W           'W'   /* 87 */
#define K_X           'X'   /* 88 */
#define K_Y           'Y'   /* 89 */
#define K_Z           'Z'   /* 90 */
#define K_bracketleft '['   /* 91 */
#define K_backslash   '\\'  /* 92 */
#define K_bracketright ']'  /* 93 */
#define K_circum      '^'   /* 94 */
#define K_underscore  '_'   /* 95 */
#define K_grave       '`'   /* 96 */
#define K_a           'a'   /* 97 (0x61) */
#define K_b           'b'   /* 98 */
#define K_c           'c'   /* 99 */
#define K_d           'd'   /* 100 */
#define K_e           'e'   /* 101 */
#define K_f           'f'   /* 102 */
#define K_g           'g'   /* 103 */
#define K_h           'h'   /* 104 */
#define K_i           'i'   /* 105 */
#define K_j           'j'   /* 106 */
#define K_k           'k'   /* 107 */
#define K_l           'l'   /* 108 */
#define K_m           'm'   /* 109 */
#define K_n           'n'   /* 110 */
#define K_o           'o'   /* 111 */
#define K_p           'p'   /* 112 */
#define K_q           'q'   /* 113 */
#define K_r           'r'   /* 114 */
#define K_s           's'   /* 115 */
#define K_t           't'   /* 116 */
#define K_u           'u'   /* 117 */
#define K_v           'v'   /* 118 */
#define K_w           'w'   /* 119 */
#define K_x           'x'   /* 120 */
#define K_y           'y'   /* 121 */
#define K_z           'z'   /* 122 */
#define K_braceleft   '{'   /* 123 */
#define K_bar         '|'   /* 124 */
#define K_braceright  '}'   /* 125 */
#define K_tilde       '~'   /* 126 */

#define iup_isprint(_c) ((_c) > 31 && (_c) < 127)

/* also define the escape sequences that have keys associated */

#define K_BS     '\b'       /* 8 */
#define K_TAB    '\t'       /* 9 */
#define K_LF     '\n'       /* 10 (0x0A) not a real key, is a combination of CR with a modifier, just to document */
#define K_CR     '\r'       /* 13 (0x0D) */

/* backward compatible definitions */
                            
#define  K_quoteleft   K_grave
#define  K_quoteright  K_apostrophe
#define  isxkey        iup_isXkey

/* IUP Extended Key Codes, range start at 128      */
/* Modifiers use 256 intervals                     */
/* These key code definitions are specific to IUP  */

#define iup_isXkey(_c)       ((_c) > 128)
#define iup_isShiftXkey(_c) (((_c) > 256)  && ((_c) < 512))
#define iup_isCtrlXkey(_c)  (((_c) > 512)  && ((_c) < 768))
#define iup_isAltXkey(_c)   (((_c) > 768)  && ((_c) < 1024))
#define iup_isSysXkey(_c)   (((_c) > 1024) && ((_c) < 1280))

#define IUPxCODE(_c)   ((_c) + 128)   /* Normal (must be above 128) */
#define IUPsxCODE(_c)  ((_c) + 256)   /* Shift (must have range to include the standard keys and the normal extended keys, so must be above 256 */
#define IUPcxCODE(_c)  ((_c) + 512)   /* Ctrl   */
#define IUPmxCODE(_c)  ((_c) + 768)   /* Alt    */
#define IUPyxCODE(_c)  ((_c) + 1024)  /* Sys (Win or Apple) */

#define IUP_NUMMAXCODES 1280          /* 5*256=1280  Normal+Shift+Ctrl+Alt+Sys */

#define K_HOME     IUPxCODE(1)                
#define K_UP       IUPxCODE(2)
#define K_PGUP     IUPxCODE(3)
#define K_LEFT     IUPxCODE(4)
#define K_MIDDLE   IUPxCODE(5)
#define K_RIGHT    IUPxCODE(6)
#define K_END      IUPxCODE(7)
#define K_DOWN     IUPxCODE(8)
#define K_PGDN     IUPxCODE(9)
#define K_INS      IUPxCODE(10)    
#define K_DEL      IUPxCODE(11)    
#define K_PAUSE    IUPxCODE(12)
#define K_ESC      IUPxCODE(13)
#define K_ccedilla IUPxCODE(14)
#define K_F1       IUPxCODE(15)
#define K_F2       IUPxCODE(16)
#define K_F3       IUPxCODE(17)
#define K_F4       IUPxCODE(18)
#define K_F5       IUPxCODE(19)
#define K_F6       IUPxCODE(20)
#define K_F7       IUPxCODE(21)
#define K_F8       IUPxCODE(22)
#define K_F9       IUPxCODE(23)
#define K_F10      IUPxCODE(24)
#define K_F11      IUPxCODE(25)
#define K_F12      IUPxCODE(26)
#define K_Print    IUPxCODE(27)
#define K_Menu     IUPxCODE(28)

#define K_acute    IUPxCODE(29) /* no Shift/Ctrl/Alt */

#define K_sHOME    IUPsxCODE(K_HOME   )
#define K_sUP      IUPsxCODE(K_UP     )
#define K_sPGUP    IUPsxCODE(K_PGUP   )
#define K_sLEFT    IUPsxCODE(K_LEFT   )
#define K_sMIDDLE  IUPsxCODE(K_MIDDLE )
#define K_sRIGHT   IUPsxCODE(K_RIGHT  )
#define K_sEND     IUPsxCODE(K_END    )
#define K_sDOWN    IUPsxCODE(K_DOWN   )
#define K_sPGDN    IUPsxCODE(K_PGDN   )
#define K_sINS     IUPsxCODE(K_INS    )
#define K_sDEL     IUPsxCODE(K_DEL    )
#define K_sSP      IUPsxCODE(K_SP     )
#define K_sTAB     IUPsxCODE(K_TAB    )
#define K_sCR      IUPsxCODE(K_CR     )
#define K_sBS      IUPsxCODE(K_BS     )
#define K_sPAUSE   IUPsxCODE(K_PAUSE  )
#define K_sESC     IUPsxCODE(K_ESC    )
#define K_Ccedilla IUPsxCODE(K_ccedilla)
#define K_sF1      IUPsxCODE(K_F1     )
#define K_sF2      IUPsxCODE(K_F2     )
#define K_sF3      IUPsxCODE(K_F3     )
#define K_sF4      IUPsxCODE(K_F4     )
#define K_sF5      IUPsxCODE(K_F5     )
#define K_sF6      IUPsxCODE(K_F6     )
#define K_sF7      IUPsxCODE(K_F7     )
#define K_sF8      IUPsxCODE(K_F8     )
#define K_sF9      IUPsxCODE(K_F9     )
#define K_sF10     IUPsxCODE(K_F10    )
#define K_sF11     IUPsxCODE(K_F11    )
#define K_sF12     IUPsxCODE(K_F12    )
#define K_sPrint   IUPsxCODE(K_Print  )
#define K_sMenu    IUPsxCODE(K_Menu   )

#define K_cHOME     IUPcxCODE(K_HOME    )
#define K_cUP       IUPcxCODE(K_UP      )
#define K_cPGUP     IUPcxCODE(K_PGUP    )
#define K_cLEFT     IUPcxCODE(K_LEFT    )
#define K_cMIDDLE   IUPcxCODE(K_MIDDLE  )
#define K_cRIGHT    IUPcxCODE(K_RIGHT   )
#define K_cEND      IUPcxCODE(K_END     )
#define K_cDOWN     IUPcxCODE(K_DOWN    )
#define K_cPGDN     IUPcxCODE(K_PGDN    )
#define K_cINS      IUPcxCODE(K_INS     )
#define K_cDEL      IUPcxCODE(K_DEL     )
#define K_cSP       IUPcxCODE(K_SP      )
#define K_cTAB      IUPcxCODE(K_TAB     )
#define K_cCR       IUPcxCODE(K_CR      )
#define K_cBS       IUPcxCODE(K_BS      )
#define K_cPAUSE    IUPcxCODE(K_PAUSE   )
#define K_cESC      IUPcxCODE(K_ESC     )
#define K_cCcedilla IUPcxCODE(K_ccedilla)
#define K_cF1       IUPcxCODE(K_F1      )
#define K_cF2       IUPcxCODE(K_F2      )
#define K_cF3       IUPcxCODE(K_F3      )
#define K_cF4       IUPcxCODE(K_F4      )
#define K_cF5       IUPcxCODE(K_F5      )
#define K_cF6       IUPcxCODE(K_F6      )
#define K_cF7       IUPcxCODE(K_F7      )
#define K_cF8       IUPcxCODE(K_F8      )
#define K_cF9       IUPcxCODE(K_F9      )
#define K_cF10      IUPcxCODE(K_F10     )
#define K_cF11      IUPcxCODE(K_F11     )
#define K_cF12      IUPcxCODE(K_F12     )
#define K_cPrint    IUPcxCODE(K_Print   )
#define K_cMenu     IUPcxCODE(K_Menu    )

#define K_mHOME     IUPmxCODE(K_HOME    )
#define K_mUP       IUPmxCODE(K_UP      )
#define K_mPGUP     IUPmxCODE(K_PGUP    )
#define K_mLEFT     IUPmxCODE(K_LEFT    )
#define K_mMIDDLE   IUPmxCODE(K_MIDDLE  )
#define K_mRIGHT    IUPmxCODE(K_RIGHT   )
#define K_mEND      IUPmxCODE(K_END     )
#define K_mDOWN     IUPmxCODE(K_DOWN    )
#define K_mPGDN     IUPmxCODE(K_PGDN    )
#define K_mINS      IUPmxCODE(K_INS     )
#define K_mDEL      IUPmxCODE(K_DEL     )
#define K_mSP       IUPmxCODE(K_SP      )
#define K_mTAB      IUPmxCODE(K_TAB     )
#define K_mCR       IUPmxCODE(K_CR      )
#define K_mBS       IUPmxCODE(K_BS      )
#define K_mPAUSE    IUPmxCODE(K_PAUSE   )
#define K_mESC      IUPmxCODE(K_ESC     )
#define K_mCcedilla IUPmxCODE(K_ccedilla)
#define K_mF1       IUPmxCODE(K_F1      )
#define K_mF2       IUPmxCODE(K_F2      )
#define K_mF3       IUPmxCODE(K_F3      )
#define K_mF4       IUPmxCODE(K_F4      )
#define K_mF5       IUPmxCODE(K_F5      )
#define K_mF6       IUPmxCODE(K_F6      )
#define K_mF7       IUPmxCODE(K_F7      )
#define K_mF8       IUPmxCODE(K_F8      )
#define K_mF9       IUPmxCODE(K_F9      )
#define K_mF10      IUPmxCODE(K_F10     )
#define K_mF11      IUPmxCODE(K_F11     )
#define K_mF12      IUPmxCODE(K_F12     )
#define K_mPrint    IUPmxCODE(K_Print   )
#define K_mMenu     IUPmxCODE(K_Menu    )

#define K_yHOME     IUPyxCODE(K_HOME    )
#define K_yUP       IUPyxCODE(K_UP      )
#define K_yPGUP     IUPyxCODE(K_PGUP    )
#define K_yLEFT     IUPyxCODE(K_LEFT    )
#define K_yMIDDLE   IUPyxCODE(K_MIDDLE  )
#define K_yRIGHT    IUPyxCODE(K_RIGHT   )
#define K_yEND      IUPyxCODE(K_END     )
#define K_yDOWN     IUPyxCODE(K_DOWN    )
#define K_yPGDN     IUPyxCODE(K_PGDN    )
#define K_yINS      IUPyxCODE(K_INS     )
#define K_yDEL      IUPyxCODE(K_DEL     )
#define K_ySP       IUPyxCODE(K_SP      )
#define K_yTAB      IUPyxCODE(K_TAB     )
#define K_yCR       IUPyxCODE(K_CR      )
#define K_yBS       IUPyxCODE(K_BS      )
#define K_yPAUSE    IUPyxCODE(K_PAUSE   )
#define K_yESC      IUPyxCODE(K_ESC     )
#define K_yCcedilla IUPyxCODE(K_ccedilla)
#define K_yF1       IUPyxCODE(K_F1      )
#define K_yF2       IUPyxCODE(K_F2      )
#define K_yF3       IUPyxCODE(K_F3      )
#define K_yF4       IUPyxCODE(K_F4      )
#define K_yF5       IUPyxCODE(K_F5      )
#define K_yF6       IUPyxCODE(K_F6      )
#define K_yF7       IUPyxCODE(K_F7      )
#define K_yF8       IUPyxCODE(K_F8      )
#define K_yF9       IUPyxCODE(K_F9      )
#define K_yF10      IUPyxCODE(K_F10     )
#define K_yF11      IUPyxCODE(K_F11     )
#define K_yF12      IUPyxCODE(K_F12     )
#define K_yPrint    IUPyxCODE(K_Print   )
#define K_yMenu     IUPyxCODE(K_Menu    )

#define K_sPlus         IUPsxCODE(K_plus    )   
#define K_sComma        IUPsxCODE(K_comma   )   
#define K_sMinus        IUPsxCODE(K_minus   )   
#define K_sPeriod       IUPsxCODE(K_period  )   
#define K_sSlash        IUPsxCODE(K_slash   )   
#define K_sAsterisk     IUPsxCODE(K_asterisk)
                        
#define K_cA     IUPcxCODE(K_A)
#define K_cB     IUPcxCODE(K_B)
#define K_cC     IUPcxCODE(K_C)
#define K_cD     IUPcxCODE(K_D)
#define K_cE     IUPcxCODE(K_E)
#define K_cF     IUPcxCODE(K_F)
#define K_cG     IUPcxCODE(K_G)
#define K_cH     IUPcxCODE(K_H)
#define K_cI     IUPcxCODE(K_I)
#define K_cJ     IUPcxCODE(K_J)
#define K_cK     IUPcxCODE(K_K)
#define K_cL     IUPcxCODE(K_L)
#define K_cM     IUPcxCODE(K_M)
#define K_cN     IUPcxCODE(K_N)
#define K_cO     IUPcxCODE(K_O)
#define K_cP     IUPcxCODE(K_P)
#define K_cQ     IUPcxCODE(K_Q)
#define K_cR     IUPcxCODE(K_R)
#define K_cS     IUPcxCODE(K_S)
#define K_cT     IUPcxCODE(K_T)
#define K_cU     IUPcxCODE(K_U)
#define K_cV     IUPcxCODE(K_V)
#define K_cW     IUPcxCODE(K_W)
#define K_cX     IUPcxCODE(K_X)
#define K_cY     IUPcxCODE(K_Y)
#define K_cZ     IUPcxCODE(K_Z)
#define K_c1     IUPcxCODE(K_1)
#define K_c2     IUPcxCODE(K_2)
#define K_c3     IUPcxCODE(K_3)
#define K_c4     IUPcxCODE(K_4)
#define K_c5     IUPcxCODE(K_5)
#define K_c6     IUPcxCODE(K_6)
#define K_c7     IUPcxCODE(K_7)        
#define K_c8     IUPcxCODE(K_8)         
#define K_c9     IUPcxCODE(K_9)
#define K_c0     IUPcxCODE(K_0)
#define K_cPlus         IUPcxCODE(K_plus        )   
#define K_cComma        IUPcxCODE(K_comma       )   
#define K_cMinus        IUPcxCODE(K_minus       )   
#define K_cPeriod       IUPcxCODE(K_period      )   
#define K_cSlash        IUPcxCODE(K_slash       )   
#define K_cSemicolon    IUPcxCODE(K_semicolon   ) 
#define K_cEqual        IUPcxCODE(K_equal       )
#define K_cBracketleft  IUPcxCODE(K_bracketleft )
#define K_cBracketright IUPcxCODE(K_bracketright)
#define K_cBackslash    IUPcxCODE(K_backslash   )
#define K_cAsterisk     IUPcxCODE(K_asterisk    )

#define K_mA     IUPmxCODE(K_A)
#define K_mB     IUPmxCODE(K_B)
#define K_mC     IUPmxCODE(K_C)
#define K_mD     IUPmxCODE(K_D)
#define K_mE     IUPmxCODE(K_E)
#define K_mF     IUPmxCODE(K_F)
#define K_mG     IUPmxCODE(K_G)
#define K_mH     IUPmxCODE(K_H)
#define K_mI     IUPmxCODE(K_I)
#define K_mJ     IUPmxCODE(K_J)
#define K_mK     IUPmxCODE(K_K)
#define K_mL     IUPmxCODE(K_L)
#define K_mM     IUPmxCODE(K_M)
#define K_mN     IUPmxCODE(K_N)
#define K_mO     IUPmxCODE(K_O)
#define K_mP     IUPmxCODE(K_P)
#define K_mQ     IUPmxCODE(K_Q)
#define K_mR     IUPmxCODE(K_R)
#define K_mS     IUPmxCODE(K_S)
#define K_mT     IUPmxCODE(K_T)
#define K_mU     IUPmxCODE(K_U)
#define K_mV     IUPmxCODE(K_V)
#define K_mW     IUPmxCODE(K_W)
#define K_mX     IUPmxCODE(K_X)
#define K_mY     IUPmxCODE(K_Y)
#define K_mZ     IUPmxCODE(K_Z)
#define K_m1     IUPmxCODE(K_1)
#define K_m2     IUPmxCODE(K_2)
#define K_m3     IUPmxCODE(K_3)
#define K_m4     IUPmxCODE(K_4)
#define K_m5     IUPmxCODE(K_5)
#define K_m6     IUPmxCODE(K_6)
#define K_m7     IUPmxCODE(K_7)        
#define K_m8     IUPmxCODE(K_8)         
#define K_m9     IUPmxCODE(K_9)
#define K_m0     IUPmxCODE(K_0)
#define K_mPlus         IUPmxCODE(K_plus        )   
#define K_mComma        IUPmxCODE(K_comma       )   
#define K_mMinus        IUPmxCODE(K_minus       )   
#define K_mPeriod       IUPmxCODE(K_period      )   
#define K_mSlash        IUPmxCODE(K_slash       )   
#define K_mSemicolon    IUPmxCODE(K_semicolon   ) 
#define K_mEqual        IUPmxCODE(K_equal       )
#define K_mBracketleft  IUPmxCODE(K_bracketleft )
#define K_mBracketright IUPmxCODE(K_bracketright)
#define K_mBackslash    IUPmxCODE(K_backslash   )
#define K_mAsterisk     IUPmxCODE(K_asterisk    )

#define K_yA     IUPyxCODE(K_A)
#define K_yB     IUPyxCODE(K_B)
#define K_yC     IUPyxCODE(K_C)
#define K_yD     IUPyxCODE(K_D)
#define K_yE     IUPyxCODE(K_E)
#define K_yF     IUPyxCODE(K_F)
#define K_yG     IUPyxCODE(K_G)
#define K_yH     IUPyxCODE(K_H)
#define K_yI     IUPyxCODE(K_I)
#define K_yJ     IUPyxCODE(K_J)
#define K_yK     IUPyxCODE(K_K)
#define K_yL     IUPyxCODE(K_L)
#define K_yM     IUPyxCODE(K_M)
#define K_yN     IUPyxCODE(K_N)
#define K_yO     IUPyxCODE(K_O)
#define K_yP     IUPyxCODE(K_P)
#define K_yQ     IUPyxCODE(K_Q)
#define K_yR     IUPyxCODE(K_R)
#define K_yS     IUPyxCODE(K_S)
#define K_yT     IUPyxCODE(K_T)
#define K_yU     IUPyxCODE(K_U)
#define K_yV     IUPyxCODE(K_V)
#define K_yW     IUPyxCODE(K_W)
#define K_yX     IUPyxCODE(K_X)
#define K_yY     IUPyxCODE(K_Y)
#define K_yZ     IUPyxCODE(K_Z)
#define K_y1     IUPyxCODE(K_1)
#define K_y2     IUPyxCODE(K_2)
#define K_y3     IUPyxCODE(K_3)
#define K_y4     IUPyxCODE(K_4)
#define K_y5     IUPyxCODE(K_5)
#define K_y6     IUPyxCODE(K_6)
#define K_y7     IUPyxCODE(K_7)        
#define K_y8     IUPyxCODE(K_8)         
#define K_y9     IUPyxCODE(K_9)
#define K_y0     IUPyxCODE(K_0)
#define K_yPlus         IUPyxCODE(K_plus        )   
#define K_yComma        IUPyxCODE(K_comma       )   
#define K_yMinus        IUPyxCODE(K_minus       )   
#define K_yPeriod       IUPyxCODE(K_period      )   
#define K_ySlash        IUPyxCODE(K_slash       )   
#define K_ySemicolon    IUPyxCODE(K_semicolon   ) 
#define K_yEqual        IUPyxCODE(K_equal       )
#define K_yBracketleft  IUPyxCODE(K_bracketleft )
#define K_yBracketright IUPyxCODE(K_bracketright)
#define K_yBackslash    IUPyxCODE(K_backslash   )
#define K_yAsterisk     IUPyxCODE(K_asterisk    )


#endif
