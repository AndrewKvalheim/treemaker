/**********************************************************************

wn_seed_random_number_generator(seed)

int wn_random_int()

int wn_random_n_bits(n)

int wn_random_bit()

**********************************************************************/
/****************************************************************************

COPYRIGHT NOTICE:

  The source code in this file is provided free of charge
  to the author's consulting clients.  It is in the
  public domain and therefore may be used by anybody for
  any purpose.

AUTHOR:

  Will Naylor

****************************************************************************/

#include <stdio.h>

#include "wnlib.h"
#include "wnmemb.h"
#include "wnrtab.h"
#include "wnrnd.h"



#define BUFFER_SIZE     1999  /* prime number */
#define WINDOW_SIZE      293  /* prime number */
#define NUM_MUNGE_FUNCS  283  /* prime number */
#define OUT_OFFSET       113

local int pointer=1656;
local int munge_pointer=70;  
local int munge_pointer_out=(70+OUT_OFFSET)%NUM_MUNGE_FUNCS;  

local int saved_bits;
local int number_of_saved_bits;

local int state[BUFFER_SIZE] =  /* for seed 0 */
{(int)0x2c1b1e0e,(int)0x841386a,(int)0xc55aca14,(int)0x2c1c3c53,(int)0xc9a2cb47,
(int)0x3574a50e,(int)0x51f0083f,(int)0xa67c732e,(int)0x7409311d,(int)0xe198093e,(int)0xa4a3535c,
(int)0xcab5dea5,(int)0x641be6da,(int)0x5056c2a,(int)0xdf31d8ba,(int)0x2cd49038,(int)0x579e65a6,
(int)0xeb734d64,(int)0x88f04b77,(int)0x8df57f7f,(int)0xeb256e8f,(int)0x5ae64c64,(int)0x483a8721,
(int)0xa72221b,(int)0xbc8630b9,(int)0x9d1f6cc7,(int)0xd6761fe4,(int)0x7f4d1ac4,(int)0xa5ce538e,
(int)0x8b266b7c,(int)0x6c954036,(int)0x74e081d,(int)0xc72fa472,(int)0xa1630313,(int)0x40c76751,
(int)0xed2b1fad,(int)0x23e4f6c8,(int)0xb40e0550,(int)0xf6309a6a,(int)0xb50f8e6a,(int)0x1104fdff,
(int)0x18c6adfd,(int)0xe7bea1e2,(int)0x78fa02d0,(int)0x6958e10f,(int)0x2afdebc3,(int)0x61f0e7a8,
(int)0x50651926,(int)0x76e3356d,(int)0x5692bd9c,(int)0xea187a29,(int)0xef6945fe,(int)0x92d3bb4c,
(int)0x4ee8c59,(int)0x69b9c7f1,(int)0x28f25f47,(int)0xda208e,(int)0xe1eb9bd8,(int)0x960918da,
(int)0x84341d94,(int)0x731fcf26,(int)0x4b4ad9ea,(int)0x239a4b9b,(int)0x920cde04,(int)0x58d0c8ce,
(int)0x5adfe504,(int)0x6cd9498f,(int)0xfa662ec6,(int)0xbf16df4,(int)0x2eeb588f,(int)0x7de69e62,
(int)0x579af0b9,(int)0x11efa44f,(int)0xeff83f1d,(int)0xbbe4987f,(int)0xb29a36d8,(int)0x68ae694f,
(int)0x1b727d91,(int)0xf73d3932,(int)0xc058c38b,(int)0x2fe5e51,(int)0xeda11a57,(int)0xdc5f716b,
(int)0xde423c35,(int)0x839b16df,(int)0xf4ac40ec,(int)0xded6c7a2,(int)0xb37ec364,(int)0xc6a782ab,
(int)0xb648f27e,(int)0xbe92019c,(int)0x4d2fcd50,(int)0x9875760f,(int)0x392a7884,(int)0x731d0425,
(int)0xda7d42ac,(int)0x778b1ef0,(int)0x1f60546e,(int)0xf7c33cd9,(int)0xe73a2dc7,(int)0xf8bf3983,
(int)0xf27b749,(int)0x6c7980d3,(int)0x6864905d,(int)0xe5537807,(int)0x39e57e38,(int)0xd2f6b144,
(int)0xc37a55f2,(int)0xd44f6492,(int)0xfcc652a2,(int)0x675b90c3,(int)0x7ce1dff5,(int)0xad84e358,
(int)0x85ab60d9,(int)0xd4dfc7df,(int)0x22a68954,(int)0x310179cd,(int)0xf92648f2,(int)0x4077ad1e,
(int)0x58a213bd,(int)0x4748b65e,(int)0x699dad8e,(int)0xf4ed2690,(int)0x7bd868f7,(int)0x4f3222b5,
(int)0x8e4ff193,(int)0x11316297,(int)0x791d14ed,(int)0xbd99425a,(int)0xf843fe15,(int)0x3a101a5b,
(int)0xa7914a6f,(int)0x866fd54f,(int)0x35947feb,(int)0x6940d385,(int)0xf78648e8,(int)0x7fae2b01,
(int)0xe151cf2,(int)0x64fabf6b,(int)0xeb3d39a1,(int)0x7eaa2971,(int)0x4453326e,(int)0xc5c3aeea,
(int)0x1965c04c,(int)0x8a355bca,(int)0x12709e4e,(int)0x8bf01abe,(int)0x226a0f5a,(int)0xfb2e46,
(int)0x5c45f9a5,(int)0x68ac40ce,(int)0xdad2ccf5,(int)0x6822a4fe,(int)0xcce1cf66,(int)0xfd61ee7,
(int)0x10fcba24,(int)0xad5603c1,(int)0x807007b,(int)0x4f7c9c0b,(int)0x49af020,(int)0xa4b7d545,
(int)0x7ece07c9,(int)0x63f484a,(int)0x546cc7d7,(int)0x4ceed730,(int)0xeadd670f,(int)0xeded556d,
(int)0x9c1c5a6f,(int)0x19b44ae4,(int)0x1dde3277,(int)0xfa31235a,(int)0x8606ead7,(int)0x77ecbd51,
(int)0x8075945f,(int)0x82cef49,(int)0x3e0a7cd9,(int)0xf4620e4f,(int)0x323db1e7,(int)0xe7c118e2,
(int)0x94adead7,(int)0x47c73301,(int)0x447be127,(int)0xfed9bc32,(int)0xfa428a6b,(int)0xb235d770,
(int)0x1d3561e2,(int)0x15981896,(int)0x3c0fea0d,(int)0xfcdae732,(int)0xe6671661,(int)0x54c5557,
(int)0xdb50fe0e,(int)0x156a0c4c,(int)0xbf3e7b22,(int)0xf006682,(int)0xb01fb91d,(int)0x5a391f2e,
(int)0x4087e33c,(int)0x3999470a,(int)0x3f119986,(int)0xb5c48ab0,(int)0x586b569d,(int)0x318995b9,
(int)0xe43f31c6,(int)0x877950ec,(int)0x14a3f362,(int)0x69263dd1,(int)0xc0b05842,(int)0x1f96dc6c,
(int)0x9812f188,(int)0xf16fa0ea,(int)0xa1e048cc,(int)0x3f4be436,(int)0xcdef8b1f,(int)0x35aad156,
(int)0xff0e1434,(int)0x7ee009a6,(int)0xa101a652,(int)0x3337abeb,(int)0x8d57dbc2,(int)0x996a0e26,
(int)0x39ddf429,(int)0xdb897ef2,(int)0xeb174310,(int)0x6f8ab58,(int)0x7713289b,(int)0x9c764bbf,
(int)0x211ce5c7,(int)0xf39f6558,(int)0x1fdd5042,(int)0x133f8616,(int)0x4de06d79,(int)0xbda63795,
(int)0x4222a2da,(int)0x8e00afb5,(int)0x500f9c0d,(int)0x74d89554,(int)0x52a6c445,(int)0x55531f25,
(int)0xd30c2f9a,(int)0xfc562f96,(int)0xb894895e,(int)0xac50f137,(int)0xbe7b03b4,(int)0xcefe5a10,
(int)0x1ccd70c7,(int)0x87b92694,(int)0x94e172e8,(int)0xc321ba49,(int)0xd5d4f405,(int)0x6e9f5581,
(int)0xb506cc68,(int)0xadd28731,(int)0x65c76e44,(int)0x260c878e,(int)0x79670d40,(int)0xb3a4f089,
(int)0xcb1c24e1,(int)0x55ceca63,(int)0x7bc3da5a,(int)0xc632e074,(int)0xbc67fdc4,(int)0x85a525bd,
(int)0x931fc5aa,(int)0x8c653085,(int)0xfbb46ed7,(int)0x659ba2a2,(int)0x8df46c0d,(int)0xb93b412b,
(int)0x9373d8d5,(int)0x62b95853,(int)0xf57329f0,(int)0x55df2a33,(int)0x2ded552f,(int)0x88db0784,
(int)0x4f959ded,(int)0x493652ec,(int)0xce59f1ed,(int)0x1233bef0,(int)0x15f56d40,(int)0x5c08b8b3,
(int)0xe0ebbfd,(int)0x9716079,(int)0x179e712d,(int)0x3f40adab,(int)0x69bb6f22,(int)0xf29403c9,
(int)0xcf1d233f,(int)0x2cb7f0a0,(int)0x2e9f545f,(int)0x97e7d8d4,(int)0x13209a3,(int)0xdd96a5ff,
(int)0x6ecc1fce,(int)0x17b8cab9,(int)0x76718c3d,(int)0xbfb4d1df,(int)0xfd6f886b,(int)0x62c99139,
(int)0x14e086c6,(int)0x98c44cc0,(int)0xcc3a3f6f,(int)0x38949935,(int)0x4d01cbbf,(int)0xd5212375,
(int)0x1cf39891,(int)0x6fe91b12,(int)0x78465d53,(int)0xd90e150e,(int)0xae3bc41a,(int)0xc8e322ac,
(int)0xe039bcad,(int)0xf87e8ecf,(int)0x241d6d09,(int)0x2ede37ef,(int)0x8759b704,(int)0x74a40dbb,
(int)0x159b6f0f,(int)0x7fa7036e,(int)0xcbd9942e,(int)0x3116eebc,(int)0x7b410921,(int)0x88e7d0b3,
(int)0x90ccb279,(int)0xf525877a,(int)0x94def541,(int)0xd824d494,(int)0x9fa6db36,(int)0xf943d8fb,
(int)0xd7f563f0,(int)0x26bb75ff,(int)0xab7729bb,(int)0xa970148f,(int)0x3ae9b881,(int)0x7ee3d31f,
(int)0xaf416c8d,(int)0x7d5c92f4,(int)0xefca48d3,(int)0xb97c754b,(int)0x8ac78a5e,(int)0xa817c725,
(int)0x9bfa4a78,(int)0xe04ad69f,(int)0xa843e889,(int)0x735e4510,(int)0xce51b811,(int)0x16f0f0c1,
(int)0xf263b23c,(int)0xed841a05,(int)0xd14a09df,(int)0xf36a518e,(int)0xe98a41d0,(int)0x8f5d9820,
(int)0xfa3992aa,(int)0xb1b3e64,(int)0x4004aaf2,(int)0x13bfb4d3,(int)0xb45cda1f,(int)0x6ab9f688,
(int)0x6deeb1b0,(int)0x99a74a53,(int)0xff49ad7f,(int)0x7f69f127,(int)0x4a25bd05,(int)0x7f8f2410,
(int)0x95613b72,(int)0x8ad61101,(int)0x14f53c4,(int)0xfcfb944c,(int)0xb13d654,(int)0xfa52e3e,
(int)0x9e925841,(int)0x28f609f3,(int)0xd7b5dd30,(int)0x4d6bcc46,(int)0x3e4c9669,(int)0x689bfe6a,
(int)0x4e91f90c,(int)0xe8504e85,(int)0x69363ce7,(int)0xa64ae750,(int)0x179a473a,(int)0x526058a,
(int)0x44dad8f6,(int)0xaa9e52ee,(int)0x45620d6,(int)0xed09a44e,(int)0xed7801da,(int)0xaaf88c0c,
(int)0x1f0ded1f,(int)0x866fe4aa,(int)0x33ad0564,(int)0x9a85b110,(int)0x4bca7a75,(int)0xb8829fb5,
(int)0x7d2dc7fe,(int)0xf57b4e82,(int)0x62467501,(int)0x5a371b1c,(int)0x13b47697,(int)0xa167c71a,
(int)0x714390af,(int)0xc72017cd,(int)0x9f9d383c,(int)0xa14eba2c,(int)0x175a237c,(int)0x4e8786e1,
(int)0xacaf4b74,(int)0x1cc8d5d6,(int)0x47a54989,(int)0x80d9eb9d,(int)0x5b0e510e,(int)0xc8d8287d,
(int)0xfa69c3f4,(int)0x879a79d,(int)0x4e98077a,(int)0x86c66b3e,(int)0xfe15ab93,(int)0x11c022e0,
(int)0x1d4f6269,(int)0x30d121d1,(int)0x1b83fba,(int)0x67426168,(int)0x67c00cba,(int)0xf51e0543,
(int)0x2d0d293d,(int)0x89ca2f7f,(int)0xe4e3c051,(int)0xe657cc26,(int)0x42f6f8cb,(int)0x5c4943f3,
(int)0x65ed6bd4,(int)0xfd392441,(int)0x7780959d,(int)0x9bcec4a6,(int)0xfe016fd,(int)0xa26aa8e3,
(int)0x57d6138b,(int)0x1c3ab85b,(int)0xf6a7b3d3,(int)0x35b1c5d,(int)0xc4ad2e24,(int)0x7f1029fb,
(int)0x6f13a4b5,(int)0xafe75905,(int)0xd87b4f2,(int)0x4a65a59d,(int)0x4e6f22a7,(int)0xad4023cb,
(int)0x3ef5b7fc,(int)0x48730a37,(int)0x83d9f96e,(int)0xb76220,(int)0xba94aa1f,(int)0x28251729,
(int)0x4bb59270,(int)0x449a527b,(int)0xefba8aed,(int)0x14c5575d,(int)0x5b750bf,(int)0xa1f29197,
(int)0x9a67463f,(int)0x87ee6406,(int)0x8e81f52e,(int)0x60327475,(int)0x1c5a1c93,(int)0xf47b4d0b,
(int)0x69e0f457,(int)0x1e8ff18,(int)0x7905cb7c,(int)0xfce0225f,(int)0xad95c285,(int)0x544b82c4,
(int)0x5b9ca15a,(int)0xb0ce6899,(int)0xaba934fe,(int)0x6c7aef78,(int)0x8ad4a1ca,(int)0xb8d15947,
(int)0xf84edc56,(int)0xf6c83a08,(int)0xe0ad5a92,(int)0xfb02d9c1,(int)0x5cc9b281,(int)0xca16c449,
(int)0x8cc37515,(int)0xa87b8af8,(int)0xe5548f2d,(int)0xb417591,(int)0xeda4693,(int)0x6c63712e,
(int)0xbc3e43fe,(int)0x69a3813f,(int)0x780323ad,(int)0x684c9e31,(int)0x6818a41,(int)0x980c9c30,
(int)0xc33d199e,(int)0xf400ec37,(int)0x461225f2,(int)0xec9ee917,(int)0xbb04deb9,(int)0x7fbeb639,
(int)0xe2a40593,(int)0xc3155f9,(int)0xab5293cf,(int)0x46a83f81,(int)0xaf32d2a0,(int)0x612e718b,
(int)0x53bb137e,(int)0x8adf79b2,(int)0x16cb5aff,(int)0x99dc01a4,(int)0xdb32336f,(int)0x69852483,
(int)0xaeac0a1,(int)0xbf456dbe,(int)0x6196a5e5,(int)0x76cf4cd6,(int)0xee387df9,(int)0xb6d24675,
(int)0xfc35f411,(int)0x14f370d9,(int)0xa247379c,(int)0x94e7a641,(int)0x5324b63d,(int)0x3758885f,
(int)0xccdba2e4,(int)0x8db0d0cb,(int)0xc3252d2e,(int)0x9f79e16b,(int)0xb0eb1c8c,(int)0x71594e7e,
(int)0xa8f90c7b,(int)0xb7b75599,(int)0xe6412c28,(int)0x22128079,(int)0xc29f7af3,(int)0x6f741a6e,
(int)0xdd29fba3,(int)0x13aded5b,(int)0x5c0dceb1,(int)0xab3abfe8,(int)0xe8cde826,(int)0x821e09f2,
(int)0x883950c8,(int)0xf07839d6,(int)0x292b7c6b,(int)0x13187d9a,(int)0x5aacb226,(int)0x99d57b3c,
(int)0x3bab90e6,(int)0x1d7fb437,(int)0x6939f643,(int)0xa1c41a55,(int)0x334c2671,(int)0xfb44ac9b,
(int)0x2e2c39e6,(int)0x55952569,(int)0xf38d8508,(int)0xa9b8334a,(int)0xb5325c1b,(int)0x3899e1a9,
(int)0x20b835f,(int)0xb30459a8,(int)0x6374637e,(int)0xce889d52,(int)0x54bbb4f5,(int)0x5e0d21e7,
(int)0x6d44dfde,(int)0xc63ba8a7,(int)0x2d692c9e,(int)0xe536f879,(int)0x135c9b1e,(int)0x5c92a5ed,
(int)0x9d4dd399,(int)0xe1ed3903,(int)0xab76aac7,(int)0x3daff285,(int)0x23202452,(int)0xda70ccd3,
(int)0x4134d97c,(int)0xb3c2d4c0,(int)0xf34635c7,(int)0xd038ab93,(int)0xa77764ae,(int)0x87b47f30,
(int)0xc44ffe41,(int)0xf1c73c02,(int)0x18c1eb72,(int)0x37c10cbf,(int)0xbfb02d2f,(int)0x1d001acc,
(int)0x7803d9be,(int)0x50c64d4f,(int)0x49ca8ec0,(int)0x94baa2a0,(int)0xad8ffc27,(int)0x6a38059c,
(int)0x3d144220,(int)0xcae25cc8,(int)0x54f85397,(int)0x14f2ca21,(int)0x13d0ca24,(int)0x352b0210,
(int)0xe4a44360,(int)0x24bde641,(int)0x74f3c2dd,(int)0xb825a399,(int)0x3c410933,(int)0x86b910a4,
(int)0x36c967b0,(int)0xd815ee4a,(int)0xac5a4b81,(int)0x1d099eb0,(int)0x11cdab9,(int)0x3aed68b9,
(int)0xe0fc02c7,(int)0x9bfb07e8,(int)0x96e67fc2,(int)0x795a1485,(int)0xd4cf4cef,(int)0x6395b40c,
(int)0x59e4c964,(int)0x6ec41009,(int)0x3d3cfbcd,(int)0x307270de,(int)0x4e8cac41,(int)0x3d836d69,
(int)0xb77cf22f,(int)0x902be269,(int)0x9caa14c0,(int)0x2a41f3d8,(int)0x51e88480,(int)0xb13e92e,
(int)0xf6c4aead,(int)0x5f6876fc,(int)0x4967780f,(int)0x4fbdacef,(int)0xf11a6a28,(int)0x24ccd6dd,
(int)0xc9a4629b,(int)0xd90fc86a,(int)0x9830b859,(int)0x326d7c60,(int)0x599464b,(int)0xb53c87f5,
(int)0x42cd9b92,(int)0x6463ede6,(int)0xe48468c1,(int)0x8979a468,(int)0x4900d0f6,(int)0x5e2948ff,
(int)0x7d92a566,(int)0x1b65c93b,(int)0x98ee9b66,(int)0x6527401e,(int)0x7a31fc05,(int)0x25b0c5ad,
(int)0x6c467b99,(int)0xf360ffa7,(int)0xe66aeb9e,(int)0x13ccc13e,(int)0xcbdc4de7,(int)0x8a169480,
(int)0x32b42c55,(int)0xa25ac6ca,(int)0x1ec4478d,(int)0xa57d5728,(int)0x2551f20b,(int)0x8a943e79,
(int)0xb6bb982f,(int)0x3ca5ae9a,(int)0xcec25b7b,(int)0xca6dce03,(int)0xd9e1694f,(int)0xf6ced28e,
(int)0x6fca9f9f,(int)0xc2dc186c,(int)0xc9d5a84e,(int)0x28e2a682,(int)0x9f3c88de,(int)0x43ac425f,
(int)0x45cd0786,(int)0x9e704020,(int)0xaf217718,(int)0x93177dd5,(int)0xa68450a8,(int)0x84b1e7ea,
(int)0xca71b9aa,(int)0xa23cf833,(int)0xcb5a9ef1,(int)0xfdf6fb18,(int)0x1912db0c,(int)0xd2392423,
(int)0xaa8c75c8,(int)0xbc59c07a,(int)0x88bdeb63,(int)0xa14e8d53,(int)0x9aef931b,(int)0x9b4dc7e9,
(int)0x3c5eebe6,(int)0xb4292bd0,(int)0x2fced367,(int)0x25a7f663,(int)0x36e09c39,(int)0x81a68bad,
(int)0x13fbada3,(int)0xca95c5ab,(int)0x82acc1bd,(int)0x6993628e,(int)0xac8fefe,(int)0x955109,
(int)0xe85c44b,(int)0x9a63aadd,(int)0x6b0faf77,(int)0xf99b22bf,(int)0x6e27198e,(int)0xda4f1b5e,
(int)0x33073e49,(int)0x420bf95c,(int)0xb92a5edd,(int)0xde1fcde7,(int)0x6bf03f4,(int)0x87498d6,
(int)0x1d2c2070,(int)0x4c926cc2,(int)0xd39c5a12,(int)0x5e368d48,(int)0x3241b0e5,(int)0x657f2c79,
(int)0x12fcf037,(int)0xb3b1f4e3,(int)0xc73dfb30,(int)0x8c9702e1,(int)0xaa988f2e,(int)0x93d84337,
(int)0x5a5b7e4,(int)0xfc7380e4,(int)0xaffc0869,(int)0x797249e9,(int)0x37028449,(int)0x31ad3ce9,
(int)0xba760a5f,(int)0x56a641bc,(int)0x9017f96f,(int)0x13dd73be,(int)0xf6c53a03,(int)0xa4694a73,
(int)0x68dab97,(int)0x5017a242,(int)0xedbf30ac,(int)0x15a7af75,(int)0xf483b9da,(int)0x686c300a,
(int)0xeeb872e,(int)0x5002e4d5,(int)0xa65bca2e,(int)0x2d00f280,(int)0xdc83e2c,(int)0x410c5c66,
(int)0x8bbf9dbc,(int)0x8b02e7e6,(int)0xe729e2e,(int)0x219518dd,(int)0x3d999348,(int)0x14903255,
(int)0x855fc95b,(int)0xb697dda5,(int)0x2045c6d,(int)0x44b3dbc2,(int)0x1b51e353,(int)0x4ecd402d,
(int)0x3e7d66ee,(int)0x96d5edb1,(int)0x7b3ccafe,(int)0x1b71bb5d,(int)0xbd2a771b,(int)0x5faa8e65,
(int)0xf1da75ac,(int)0x3e549ad1,(int)0x73a47c29,(int)0x4aa7d479,(int)0xbb0be329,(int)0x1a089a,
(int)0xdc3f9b4,(int)0xf8844323,(int)0xbcef2203,(int)0x617dc2cc,(int)0xabb3ffa7,(int)0x72a5e329,
(int)0x6a45bdd2,(int)0xdb0c9f93,(int)0x7e85a058,(int)0x471723e9,(int)0x6c2725a3,(int)0xcca3372f,
(int)0x4fa33f4,(int)0x12cd289f,(int)0x50d88fbe,(int)0x9c6e12f2,(int)0x74dc10c9,(int)0xb159c09f,
(int)0xc4515453,(int)0xe7a7050e,(int)0x29b7766b,(int)0xdcc7ab70,(int)0x2e70d162,(int)0x46ff40c8,
(int)0x8a896386,(int)0xac460182,(int)0xdf0b392e,(int)0x6efb1c32,(int)0xcf55200d,(int)0x50e9009,
(int)0x728b6958,(int)0x1ae64f80,(int)0xd2154b66,(int)0x27b6bcb2,(int)0x49cbfb5e,(int)0x2a88dc2c,
(int)0xd5d01cf6,(int)0x2f74abbe,(int)0x2612646e,(int)0x5ef618d1,(int)0x9cf31f85,(int)0x76767ce,
(int)0x7d3e6218,(int)0x887af8cd,(int)0x273741bd,(int)0x728d4671,(int)0x5819ee8a,(int)0x39d99467,
(int)0xb1fa0366,(int)0x3140038d,(int)0x540c338a,(int)0x2761d396,(int)0x137cb2c3,(int)0xb275560f,
(int)0xfaf8bc42,(int)0x97fc42b2,(int)0xddd997b9,(int)0x7f52b2f4,(int)0xbe5082a3,(int)0x455655cf,
(int)0xfd63a080,(int)0xed693feb,(int)0xe21f3f23,(int)0x5c812f34,(int)0xa875f96e,(int)0x727fd197,
(int)0x1e6bf309,(int)0x245f27e0,(int)0xad1a65b4,(int)0x7e9fdab1,(int)0xafab4f6,(int)0x7239a0cc,
(int)0x76ce6b94,(int)0x2a04ccaf,(int)0xe9d5a3d5,(int)0x462cd1a3,(int)0x115f3471,(int)0x92c331b0,
(int)0x8bf9515d,(int)0x73bb391e,(int)0xc5dae677,(int)0x4026331f,(int)0x39df92cf,(int)0xa6349118,
(int)0xa6ecbf07,(int)0x9e809c38,(int)0x4a39080d,(int)0x586312f6,(int)0xa47bbdd,(int)0x333608e1,
(int)0xbcb52eae,(int)0xdc0e960f,(int)0xcd92f63e,(int)0xb7013119,(int)0xe0a5de4d,(int)0xd6a8989,
(int)0x1cc21756,(int)0x9eefba5,(int)0x21a16fb6,(int)0x1bd9e890,(int)0xf14b0858,(int)0xb5f5d3d7,
(int)0x2b60a387,(int)0xe8c9cf30,(int)0xb94e0f7f,(int)0xf96e611f,(int)0x77b56699,(int)0x6ef285cf,
(int)0x373d0936,(int)0x1de17dec,(int)0xc5caadbc,(int)0x886b068,(int)0xd01fb4fa,(int)0xe99f9eeb,
(int)0x4ac313cc,(int)0x5682cb74,(int)0xfb07d695,(int)0xb2c84685,(int)0x7bf80c71,(int)0x8ee3f421,
(int)0xf1bd1a08,(int)0x7c9ae9fe,(int)0xb298b486,(int)0x4ccde84d,(int)0x839234f,(int)0xedf31cb0,
(int)0x352be4c1,(int)0x8af10d8c,(int)0x2247b9fb,(int)0x635cc847,(int)0xe6421e26,(int)0x81027776,
(int)0xd2cb942b,(int)0x5a121,(int)0x24e8108d,(int)0x9ed6288e,(int)0x7bbdae1f,(int)0x1dc0cb36,
(int)0x531649a6,(int)0x55c87d1f,(int)0x84ed1bed,(int)0x2f40f524,(int)0xcb1bb99a,(int)0xdfac45a9,
(int)0x64d14c36,(int)0x2ab7a6a5,(int)0xee02053b,(int)0x3c01ae5c,(int)0x1f53b426,(int)0x34055072,
(int)0x2f4707b,(int)0x61105419,(int)0x54a35a0c,(int)0x9ee611cd,(int)0xf4ff00a6,(int)0x76ca5e87,
(int)0xd06d035b,(int)0x10d00735,(int)0x2eb8cef9,(int)0x5fcc7719,(int)0xeedf0ccc,(int)0x4b40052a,
(int)0x357820c9,(int)0x20e74bff,(int)0xebda850e,(int)0x812bfa9e,(int)0x5ce4d7ea,(int)0xcb8792c9,
(int)0x3c6ff276,(int)0x807b3269,(int)0x46587a40,(int)0xfce50616,(int)0x63508dc1,(int)0xc6526b24,
(int)0xb59319a,(int)0x8df6e98c,(int)0xbfa134a8,(int)0x1a0599d4,(int)0xbea981a8,(int)0x1f6c808c,
(int)0x6680ebb1,(int)0xc2f4fdbe,(int)0xba506712,(int)0xd8041d99,(int)0x29d26a50,(int)0x648de5f0,
(int)0x8719d343,(int)0xccc3cccc,(int)0x320c302,(int)0xc15bea50,(int)0xda11bf12,(int)0x1bf4b50c,
(int)0xc7473210,(int)0x182df4b5,(int)0x622a137b,(int)0xaa013065,(int)0x49c834b3,(int)0xe84b505f,
(int)0x5468ef25,(int)0xb6873741,(int)0xcbbd4c0d,(int)0x3d0af0f8,(int)0x2f6bbc60,(int)0xa9406503,
(int)0x3cbbd491,(int)0x6f52a39e,(int)0xe3963869,(int)0xa0134522,(int)0x714a4a08,(int)0xc59f2103,
(int)0x869b5f73,(int)0xae405913,(int)0xa195bfcb,(int)0x91a75dd0,(int)0x84e9aff5,(int)0x370e854d,
(int)0x483891d3,(int)0x8e923940,(int)0xa47e6be,(int)0x2903ccdd,(int)0xae989ae0,(int)0x254ffbb,
(int)0xbe01bfe0,(int)0xb0091994,(int)0x1db6fa36,(int)0xb552d4ec,(int)0x948d7de2,(int)0x94db28df,
(int)0xa2c84565,(int)0x57afe9a7,(int)0x9e9ebc05,(int)0xbadd33df,(int)0xbc0f16b,(int)0x3509aa3e,
(int)0x8cc3b032,(int)0xd3898bd6,(int)0x71a0ab5d,(int)0x363e8292,(int)0xb00d9517,(int)0x87ac796e,
(int)0x918288a6,(int)0x699e089d,(int)0x891a633e,(int)0xb7d41f6f,(int)0x5f9cd392,(int)0xf0937b8f,
(int)0xafe13839,(int)0x59180af,(int)0x71dda0e7,(int)0xfd5ab1f5,(int)0x6953160f,(int)0x10386eff,
(int)0xbf65d56b,(int)0x29ee10b4,(int)0x48182aa0,(int)0xbfc332c,(int)0x59376bd0,(int)0x90d04c3f,
(int)0x81928c3e,(int)0x5d6f0f64,(int)0xaaf3b2da,(int)0x1f3ed88a,(int)0xd1feeaba,(int)0x1c558769,
(int)0x7d11bf2f,(int)0xbb4c1a44,(int)0xe4d6a039,(int)0x40b2d75,(int)0x3e7af0c0,(int)0xed415bd,
(int)0xcc9611dd,(int)0xa0c1bf0e,(int)0x8f614c4a,(int)0xb3f39d77,(int)0xe4adce23,(int)0x97980a60,
(int)0x1d03684,(int)0xcf7d9137,(int)0x3ca49470,(int)0x5bf41eca,(int)0x7f5bddb6,(int)0x66d2ffe0,
(int)0xb4b7eb85,(int)0xe64de6a8,(int)0x8d7c87fc,(int)0x4160f878,(int)0xc33fcdb8,(int)0x80866e6c,
(int)0xae4e4ea,(int)0x6a268276,(int)0x9f42a654,(int)0x475e3ea2,(int)0x109d1164,(int)0x33d3c7ad,
(int)0x300ddf5b,(int)0xb46b1246,(int)0x549dcac,(int)0x6af2bdff,(int)0xeeafc63,(int)0x29786386,
(int)0xaf7006a1,(int)0x754b58b,(int)0xc1ae9668,(int)0xb1a5cd45,(int)0x3573fda7,(int)0x3e0a6ef9,
(int)0xaa25d144,(int)0xe7798d41,(int)0xbaf59882,(int)0x57e15553,(int)0xf802a56,(int)0x24734977,
(int)0xb6b5abf,(int)0x34422cf4,(int)0x3a9a17d3,(int)0xcec0a697,(int)0xea779e1d,(int)0x7f8336be,
(int)0xb6120e4e,(int)0xcc926db9,(int)0x4d9378ec,(int)0x9c024388,(int)0xa1ed9eaa,(int)0x566cc69c,
(int)0x3dc3fc60,(int)0xe15bae9b,(int)0xaaabb166,(int)0xf7c6d90,(int)0xd9f76265,(int)0xf4e738c,
(int)0x18f76eb1,(int)0x588a6d28,(int)0xee3c387,(int)0xbb3a3efd,(int)0x90f9da88,(int)0xb53a8f76,
(int)0xe687f2f8,(int)0x75c42cab,(int)0x4a29a802,(int)0x1dee9234,(int)0x84b819bf,(int)0xf19d258a,
(int)0xf274cece,(int)0x529d6fa5,(int)0x75a993c5,(int)0x95609349,(int)0x5b7c7668,(int)0xaff86a9f,
(int)0x435ce1b7,(int)0x413f4793,(int)0xac3e7655,(int)0x1a9e537e,(int)0x84227a42,(int)0x653402f3,
(int)0xac34e45e,(int)0x8e4999ff,(int)0xa8d19aa9,(int)0xef6a0d68,(int)0xaaf3e815,(int)0xaf0e3bfa,
(int)0x462662f8,(int)0x39162452,(int)0xe07c7150,(int)0x94c4a66b,(int)0x4758a933,(int)0x50b9c6eb,
(int)0xd5713d5f,(int)0xbf666018,(int)0x28d5531,(int)0xf76aa534,(int)0xf5729ef6,(int)0xd2574609,
(int)0x498375d5,(int)0xc5e9cf2e,(int)0x60a96789,(int)0x5be9c141,(int)0x94241a8b,(int)0xd5fdfcc4,
(int)0xd2fae57a,(int)0xa91046dd,(int)0x17bde219,(int)0x58b35a56,(int)0xf765ca66,(int)0x1859115e,
(int)0x2072f7d3,(int)0xea880557,(int)0x93c6e2f3,(int)0x3d204548,(int)0xbb76ee7b,(int)0xf04c9400,
(int)0x9ab5eba4,(int)0x1681ee9a,(int)0x25e4c350,(int)0x3950adf7,(int)0x3be40703,(int)0x58db565c,
(int)0xb14d2163,(int)0x598739eb,(int)0x69e52694,(int)0xc6872de4,(int)0x2d49f0c8,(int)0xf93b3159,
(int)0xdc053632,(int)0x9121735e,(int)0x22846180,(int)0x6287359b,(int)0x9df44e65,(int)0xa024d9e8,
(int)0x31fea7b6,(int)0xe9b15603,(int)0xf2989f9,(int)0x1b43c4f9,(int)0x5cc56d31,(int)0xd52cab45,
(int)0xe4a63ba1,(int)0xa8ac7ce9,(int)0x2594295a,(int)0xf48f306f,(int)0xfb7f82b5,(int)0xdb43a5c7,
(int)0x406bc0f8,(int)0x11a8531e,(int)0xd82869a9,(int)0x55a7b38f,(int)0x369006c8,(int)0xd760a813,
(int)0x55794753,(int)0x659cf42c,(int)0x7e2fb40e,(int)0x269347cb,(int)0x5fa53a8e,(int)0xc7de493e,
(int)0x73af8026,(int)0x6880c741,(int)0x711368da,(int)0x5188d4f4,(int)0xb60d45fd,(int)0x748c7212,
(int)0x1b8cc0d1,(int)0x87d92b19,(int)0x8a90a3a0,(int)0xff079954,(int)0xfbaf6c70,(int)0x757ab475,
(int)0x1bf12411,(int)0xf25a03c0,(int)0x973604a5,(int)0xa79f4c33,(int)0xd36cee78,(int)0x1b1cc27b,
(int)0x28536b79,(int)0x43c7b94d,(int)0x48de9fb7,(int)0x583ed404,(int)0x195ad599,(int)0xf3fb1838,
(int)0x4e1259f1,(int)0x25775860,(int)0x729008d4,(int)0xa249b6c8,(int)0xbc57a546,(int)0x7ed22d7d,
(int)0x71b45459,(int)0xb7ee8425,(int)0x8d30d24b,(int)0xf1dc88b,(int)0x8755efa9,(int)0x4f698105,
(int)0x9e82eacc,(int)0x8e4f7b2e,(int)0xff590d46,(int)0x11d97a39,(int)0x8bc9bc58,(int)0xabfe240f,
(int)0xcde0cddd,(int)0x58bdb1c5,(int)0x3bd37ac0,(int)0xd94f4c4d,(int)0x50698119,(int)0x4cd58249,
(int)0xb05bf380,(int)0x9a505c24,(int)0x3b2b8af4,(int)0x6b1c351c,(int)0x234ae643,(int)0x4d48e3cf,
(int)0x4eca95b0,(int)0xf0f5a071,(int)0x6dd47abd,(int)0x4943285d,(int)0xf66640f6,(int)0x920e1081,
(int)0xaf9b7bbc,(int)0xfee75abc,(int)0x78eb06e2,(int)0x764bbf64,(int)0x5f36c2e9,(int)0x9faafc6d,
(int)0x45389de3,(int)0x48c6d83f,(int)0xd81013b0,(int)0x286602d7,(int)0xfcf68b1d,(int)0xadfc149d,
(int)0xab25e8ca,(int)0xd2d8e32f,(int)0x32189edf,(int)0x5f29d440,(int)0xda9c12c3,(int)0xa7932a6d,
(int)0x30db406e,(int)0xb8f2275c,(int)0x9591bad2,(int)0x6b40bbff,(int)0x45d8934c,(int)0x842c1c1d,
(int)0xe7297bbd,(int)0x57517446,(int)0xeb8c51a3,(int)0xce22c467,(int)0xeaf8e79c,(int)0xb5595059,
(int)0x477bd50e,(int)0x3377476d,(int)0x6a4feb54,(int)0x48206826,(int)0x64093c58,(int)0xfd3c174e,
(int)0x8e5a5603,(int)0xf4fc1156,(int)0xa71ff38a,(int)0x27aeeade,(int)0x365110ee,(int)0xef59997c,
(int)0x767cd1b0,(int)0x588cf6cf,(int)0x8de9c580,(int)0x1688c63a,(int)0xb88464d4,(int)0x68c7c830,
(int)0xead21b80,(int)0x627c07e5,(int)0xb31f7c91,(int)0xb559803,(int)0xfd92dcf3,(int)0x8ad76b68,
(int)0xff49cef8,(int)0xcc833d9c,(int)0x7d8a0779,(int)0x91587490,(int)0x16494121,(int)0x9cb54255,
(int)0xa6656707,(int)0x7865eae2,(int)0x6d1065e7,(int)0xd24ce2c0,(int)0x7baa881,(int)0xad23b2b2,
(int)0x513df0a5,(int)0xf28cb29d,(int)0xfcee8137,(int)0xc68b39b4,(int)0xf6ce86d9,(int)0x58157ce3,
(int)0xbf8f6da2,(int)0x3039a0c5,(int)0xca9f94b9,(int)0xdbb69d55,(int)0x10b54acf,(int)0x2ef389d9,
(int)0x62d84107,(int)0xba2a5ad,(int)0x510216b5,(int)0x884472f7,(int)0x9e828ccf,(int)0xb0376695,
(int)0x1948cce8,(int)0x8c6b3c8f,(int)0xea0251f5,(int)0xa29d1ef1,(int)0x51e08144,(int)0x50d0fac9,
(int)0x66daec63,(int)0x3536dfa9,(int)0x9dd9c36,(int)0xa67444b1,(int)0x9e26d704,(int)0x1e807e7b,
(int)0x4b09197e,(int)0x3fc44902,(int)0xb005ffc6,(int)0xcac2e31e,(int)0xe6b2a531,(int)0xe58ee041,
(int)0x3d17f0c4,(int)0x1232f35e,(int)0x5f0a6d5c,(int)0xe78fc73,(int)0xf12d9e23,(int)0x7f7b3d0b,
(int)0x39b7fb23,(int)0x8fe60153,(int)0x5998168,(int)0x7b663595,(int)0xd80b1029,(int)0x82901850,
(int)0x79cfbce9,(int)0x626983b5,(int)0xb3539ae2,(int)0xcbd5147d,(int)0xf8fdc37a,(int)0x8908867a,
(int)0x16f00dae,(int)0xac6e0a45,(int)0xc5f122c1,(int)0x460d5fe1,(int)0x63ccf511,(int)0x634f412,
(int)0xf25bda5b,(int)0x24ae8c85,(int)0xee768b7a,(int)0xd0ff3ce0,(int)0x8cfaa6b7,(int)0xef2a29f0,
(int)0xa3809170,(int)0xee509abb,(int)0xaba7ad94,(int)0x60b17c1d,(int)0x33756cae,(int)0xee2ca647,
(int)0xedfdaee,(int)0xcc4d51c9,(int)0x9582e7f2,(int)0xd48fc535,(int)0xbf01db70,(int)0xa9611166,
(int)0xdda8bce5,(int)0x7c911e0f,(int)0xe184fcc7,(int)0x9498a823,(int)0xa632ebc1,(int)0x77d42fdc,
(int)0xea13387e,(int)0x2760411f,(int)0xae60d8ae,(int)0x58833525,(int)0x421865d6,(int)0xa3e0d320,
(int)0x417f6ae,(int)0x7c23febd,(int)0xf729bed9,(int)0x7b0c8352,(int)0x93f2c8ea,(int)0x47233a0d,
(int)0xf032b37a,(int)0x5cc4100d,(int)0x3175cfd6,(int)0x9abd82d5,(int)0xf358c21e,(int)0xe8421f1c,
(int)0xebcd3561,(int)0x27b7c706,(int)0x763f402a,(int)0xa39059a3,(int)0xb25d6e4,(int)0xd5343856,
(int)0xde3641f6,(int)0x7c00e19d,(int)0xaf380a8e,(int)0x4c5eb807,(int)0x5c456187,(int)0x3ab0a605,
(int)0x1a20a7b3,(int)0xa265e223,(int)0x25be8963,(int)0x35ebcb43,(int)0xd5256ca5,(int)0x32a4bb2a,
(int)0x3c49efde,(int)0xadd938ed,(int)0x76244170,(int)0xa8a93da9,(int)0x82b6aeae,(int)0xe5d2ee2,
(int)0x9f6465cb,(int)0x7315ead8,(int)0xb93ea07,(int)0xeabbe05a,(int)0x8cf3098c,(int)0x7aa1810f,
(int)0x78f0973d,(int)0x5eb0d8db,(int)0xacab45fd,(int)0x598deec4,(int)0x7b62ab37,(int)0x6862ec55,
(int)0xe029de61,(int)0xd2bd2279,(int)0x8633d82b,(int)0xdf8e5f2d,(int)0xe5a4b9b7,(int)0xb2e77077,
(int)0x16e4b3ad,(int)0x6684ed01,(int)0x512ca531,(int)0x9a3cd6a8,(int)0xf7932f06,(int)0xa11ad895,
(int)0x7bf89c95,(int)0x2a46b12d,(int)0x25599b3c,(int)0x33a14888,(int)0x92626f83,(int)0x3a31fdfa,
(int)0x1abe848d,(int)0xc284aab1,(int)0xe30bad61,(int)0xe3977be2,(int)0xba032864,(int)0xc9aa9cad,
(int)0xe625ef2a,(int)0x89e57551,(int)0xee2b0ae7,(int)0x2a390a3f,(int)0x6eb69cf5,(int)0xa752e2bc,
(int)0x13f624d,(int)0x9e04f3be,(int)0xf7e018ff,(int)0xafbead92,(int)0x7481a010,(int)0x7d3efbf1,
(int)0x92793e8,(int)0xafaca411,(int)0xeaf0bb2a,(int)0x45bf3543,(int)0x5aa0a20f,(int)0x68278ca,
(int)0x3d6f267b,(int)0x60466ec5,(int)0x627810a1,(int)0x74022af5,(int)0x8dd5a17,(int)0x78a7ee1b,
(int)0xe46a9092,(int)0xef4c5687,(int)0x336f3f36,(int)0x61c31745,(int)0xe615f122,(int)0xc830e48b,
(int)0xd23bf007,(int)0xfe385d14,(int)0xa05c6f4a,(int)0xdf4b5229,(int)0xdbac23a,(int)0x45d98095,
(int)0x2a42c45c,(int)0x8a5ca00f,(int)0x71e624a7,(int)0x5ecec8e0,(int)0xc9e6f4f2,(int)0x765e2179,
(int)0xc085354c,(int)0x221c1304,(int)0x9b3b60,(int)0xa508c9b2,(int)0x8f7a33d0,(int)0xce520be1,
(int)0xeec10b1d,(int)0x72141063,(int)0xcc86a5d9,(int)0x67113354,(int)0x212d10e2,(int)0x7952c875,
(int)0x15693b49,(int)0x8e74a003,(int)0xd6dadeae,(int)0x68673b18,(int)0xbdd0135c,(int)0x5a690bca,
(int)0xd2cf941e,(int)0x16c08ade,(int)0x604ba7ee,(int)0xb216d592,(int)0xaf63734e,(int)0xd25e694f,
(int)0x99f6a146,(int)0xd5f77117,(int)0x5a5c5f8a,(int)0x19e62622,(int)0x4f93a2a1,(int)0x530f9d06,
(int)0x7b2c2f26,(int)0xb167eeaf,(int)0x5bcb0816,(int)0x9eae693f,(int)0x71165a56,(int)0x4a2676ca,
(int)0x86923800,(int)0xdb43e4dc,(int)0x795c99c9,(int)0xd3d296fc,(int)0x9638b669,(int)0xed639d89,
(int)0x833fcb8e,(int)0x859fa3b0,(int)0x3b8be954,(int)0x89f84a5c,(int)0x214d86ed,(int)0xa95e02ef,
(int)0xe714b9c3,(int)0x9958aa0e,(int)0x8cc90063,(int)0x3b1e7b9a,(int)0x6ba52e03,(int)0x81b07b1b,
(int)0x3d4b609b,(int)0xebd7a8de,(int)0x48ae14f9,(int)0x6120009a,(int)0xef55a934,(int)0x51ab5675,
(int)0xd1b507ec,(int)0xd438298a,(int)0x37ae44d3,(int)0xd108ac28,(int)0xfc808389,(int)0xd927ad51,
(int)0x7c4a863,(int)0xae09884c,(int)0x1302f9e3,(int)0xdfb83def,(int)0x246da88,(int)0xf06c4534,
(int)0x62797727,(int)0xd415fad9,(int)0xde7de34d,(int)0x3ded25f6,(int)0x7b409214,(int)0xed12c149,
(int)0x6428d7bd,(int)0x99c809e7,(int)0x566fec42,(int)0x563dcd61,(int)0xfaad109b,(int)0x3f6a2d8c,
(int)0x5bd18b63,(int)0x27fc8d3,(int)0xd428624a,(int)0x52d0dfb7,(int)0xc964f7fd,(int)0x9cff238a,
(int)0x8ba6ffc,(int)0xe1326668,(int)0x7934df5f,(int)0x852651a5,(int)0x945ba97f,(int)0x7581479,
(int)0x34287cfd,(int)0x17b4b881,(int)0xfdcc3636,(int)0x2e2da55,(int)0x6647f8a5,(int)0x9fb72595,
(int)0x3684dcf2,(int)0x4b57059,(int)0xf5c211ce,(int)0xb95e84cd,(int)0x197b0753,(int)0x93ba8979,
(int)0xef37c9de,(int)0x83730949,(int)0xc07ae5ac,(int)0x34084f91,(int)0xc3292b81,(int)0xea9626e3,
(int)0x44697638,(int)0xe8e4e1fc,(int)0xb50b1a99,(int)0xdf79bf3d,(int)0x67efe55d,(int)0xd568db9d,
(int)0x4a9f7595,(int)0x312814fd,(int)0x9e2a747e,(int)0x4831f271,(int)0x16ca8d9f,(int)0xde0f58d0,
(int)0xe5fd817b,(int)0xdc6ed645,(int)0xaf4e409a,(int)0xb6a5b97f,(int)0x2f1eb4cf,(int)0xa39e62ac,
(int)0x76022d5a,(int)0x2d820bec,(int)0x90fb640c,(int)0x5610da09,(int)0x2073035e,(int)0x39d1d4d7,
(int)0xc0b788af,(int)0x190302d7,(int)0x49e7c1eb,(int)0xbf9a2078,(int)0x368266e8,(int)0x6e266d67,
(int)0x959a5882,(int)0xdbd78b40,(int)0xc389d7f,(int)0x15a581a8,(int)0xdd127a6c,(int)0xa05d742,
(int)0xcda3269d,(int)0xfcb2a062,(int)0x7af3f7a9,(int)0x9f64cda0,(int)0x2a518ffd,(int)0x5c02d683,
(int)0x79eed7a5,(int)0xd77e07d0,(int)0x866e6eb5,(int)0x777fa79e,(int)0x841386a,(int)0xc55aca14,
(int)0x2c1c3c53,(int)0xc9a2cb47,(int)0x3574a50e,(int)0x51f0083f,(int)0xa67c732e,(int)0x7409311d,
(int)0xe198093e,(int)0xa4a3535c,(int)0xcab5dea5,(int)0x641be6da,(int)0x5056c2a,(int)0xdf31d8ba,
(int)0x2cd49038,(int)0x579e65a6,(int)0xeb734d64,(int)0x88f04b77,(int)0x8df57f7f,(int)0xeb256e8f,
(int)0x5ae64c64,(int)0x483a8721,(int)0xa72221b,(int)0xbc8630b9,(int)0x9d1f6cc7,(int)0xd6761fe4,
(int)0x7f4d1ac4,(int)0xa5ce538e,(int)0x8b266b7c,(int)0x6c954036,(int)0x74e081d,(int)0xc72fa472,
(int)0xa1630313,(int)0x40c76751,(int)0xed2b1fad,(int)0x23e4f6c8,(int)0xb40e0550,(int)0xf6309a6a,
(int)0xb50f8e6a,(int)0x1104fdff,(int)0x18c6adfd,(int)0xe7bea1e2,(int)0x78fa02d0,(int)0x6958e10f,
(int)0x2afdebc3,(int)0x61f0e7a8,(int)0x50651926,(int)0x76e3356d,(int)0x5692bd9c,(int)0xea187a29,
(int)0xef6945fe,(int)0x92d3bb4c,(int)0x4ee8c59,(int)0x69b9c7f1,(int)0x28f25f47,(int)0xda208e,
(int)0xe1eb9bd8,(int)0x960918da,(int)0x84341d94,(int)0x731fcf26,(int)0x4b4ad9ea,(int)0x239a4b9b,
(int)0x920cde04,(int)0x58d0c8ce,(int)0x5adfe504,(int)0x6cd9498f,(int)0xfa662ec6,(int)0xbf16df4,
(int)0x2eeb588f,(int)0x7de69e62,(int)0x579af0b9,(int)0x11efa44f,(int)0xeff83f1d,(int)0xbbe4987f,
(int)0xb29a36d8,(int)0x68ae694f,(int)0x1b727d91,(int)0xf73d3932,(int)0xc058c38b,(int)0x2fe5e51,
(int)0xeda11a57,(int)0xdc5f716b,(int)0xde423c35,(int)0x839b16df,(int)0xf4ac40ec,(int)0xded6c7a2,
(int)0xb37ec364,(int)0xc6a782ab,(int)0xb648f27e,(int)0xbe92019c,(int)0x4d2fcd50,(int)0x9875760f,
(int)0x392a7884,(int)0x731d0425,(int)0xda7d42ac,(int)0x778b1ef0,(int)0x1f60546e,(int)0xf7c33cd9,
(int)0xe73a2dc7,(int)0xf8bf3983,(int)0xf27b749,(int)0x6c7980d3,(int)0x6864905d,(int)0xe5537807,
(int)0x39e57e38,(int)0xd2f6b144,(int)0xc37a55f2,(int)0xd44f6492,(int)0xfcc652a2,(int)0x675b90c3,
(int)0x7ce1dff5,(int)0xad84e358,(int)0x85ab60d9,(int)0xd4dfc7df,(int)0x22a68954,(int)0x310179cd,
(int)0xf92648f2,(int)0x4077ad1e,(int)0x58a213bd,(int)0x4748b65e,(int)0x699dad8e,(int)0xf4ed2690,
(int)0x7bd868f7,(int)0x4f3222b5,(int)0x8e4ff193,(int)0x11316297,(int)0x791d14ed,(int)0xbd99425a,
(int)0xf843fe15,(int)0x3a101a5b,(int)0xa7914a6f,(int)0x866fd54f,(int)0x35947feb,(int)0x6940d385,
(int)0xf78648e8,(int)0x7fae2b01,(int)0xe151cf2,(int)0x64fabf6b,(int)0xeb3d39a1,(int)0x7eaa2971,
(int)0x4453326e,(int)0xc5c3aeea,(int)0x1965c04c,(int)0x8a355bca,(int)0x12709e4e,(int)0x8bf01abe,
(int)0x226a0f5a,(int)0xfb2e46,(int)0x5c45f9a5,(int)0x68ac40ce,(int)0xdad2ccf5,(int)0x6822a4fe,
(int)0xcce1cf66,(int)0xfd61ee7,(int)0x10fcba24,(int)0xad5603c1,(int)0x807007b,(int)0x4f7c9c0b,
(int)0x49af020,(int)0xa4b7d545,(int)0x7ece07c9,(int)0x63f484a,(int)0x546cc7d7,(int)0x4ceed730,
(int)0xeadd670f,(int)0xeded556d,(int)0x9c1c5a6f,(int)0x19b44ae4,(int)0x1dde3277,(int)0xfa31235a,
(int)0x8606ead7,(int)0x77ecbd51,(int)0x8075945f,(int)0x82cef49,(int)0x3e0a7cd9,(int)0xf4620e4f,
(int)0x323db1e7,(int)0xe7c118e2,(int)0x94adead7,(int)0x47c73301,(int)0x447be127,(int)0xfed9bc32,
(int)0xfa428a6b,(int)0xb235d770,(int)0x1d3561e2,(int)0x15981896,(int)0x3c0fea0d,(int)0xfcdae732,
(int)0xe6671661,(int)0x54c5557,(int)0xdb50fe0e,(int)0x156a0c4c,(int)0xbf3e7b22,(int)0xf006682,
(int)0xb01fb91d,(int)0x5a391f2e,(int)0x4087e33c,(int)0x3999470a,(int)0x3f119986,(int)0xb5c48ab0,
(int)0x586b569d,(int)0x318995b9,(int)0xe43f31c6,(int)0x877950ec,(int)0x14a3f362,(int)0x69263dd1,
(int)0xc0b05842,(int)0x1f96dc6c,(int)0x9812f188,(int)0xf16fa0ea,(int)0xa1e048cc,(int)0x3f4be436,
(int)0xcdef8b1f,(int)0x35aad156,(int)0xff0e1434,(int)0x7ee009a6,(int)0xa101a652,(int)0x3337abeb,
(int)0x8d57dbc2,(int)0x996a0e26,(int)0x39ddf429,(int)0xdb897ef2,(int)0xeb174310,(int)0x6f8ab58,
(int)0x7713289b,(int)0x9c764bbf,(int)0x211ce5c7,(int)0xf39f6558,(int)0x1fdd5042,(int)0x133f8616,
(int)0x4de06d79,(int)0xbda63795,(int)0x4222a2da,(int)0x8e00afb5,(int)0x500f9c0d,(int)0x74d89554,
(int)0x52a6c445,(int)0x55531f25,(int)0xd30c2f9a,(int)0xfc562f96,(int)0xb894895e,(int)0xac50f137,
(int)0xbe7b03b4,(int)0xcefe5a10,(int)0x1ccd70c7,(int)0x87b92694,(int)0x94e172e8,(int)0xc321ba49,
(int)0xd5d4f405,(int)0x6e9f5581,(int)0xb506cc68,(int)0xadd28731,(int)0x65c76e44,(int)0x260c878e,
(int)0x79670d40,(int)0xb3a4f089,(int)0xcb1c24e1,(int)0x55ceca63,(int)0x7bc3da5a,(int)0xc632e074,
(int)0xbc67fdc4,(int)0x85a525bd,(int)0x931fc5aa,(int)0x8c653085,(int)0xfbb46ed7,(int)0x659ba2a2,
(int)0x8df46c0d,(int)0xb93b412b,(int)0x9373d8d5,(int)0x62b95853,(int)0xf57329f0,(int)0x55df2a33,
(int)0x2ded552f,(int)0x88db0784,(int)0x4f959ded,(int)0x493652ec,(int)0xce59f1ed,(int)0x1233bef0,
(int)0x15f56d40,(int)0x5c08b8b3,(int)0xe0ebbfd,(int)0x9716079,(int)0x179e712d,(int)0x3f40adab,
(int)0x69bb6f22,(int)0xf29403c9,(int)0xcf1d233f,(int)0x2cb7f0a0,(int)0x2e9f545f,(int)0x97e7d8d4,
(int)0x13209a3,(int)0xdd96a5ff};


local int munge0(register int *p)
{
  register int ret;

  ret = 1579325736;

/* linear taps */
  ret ^= p[79];
  ret -= p[154];
  ret ^= p[122];
  ret ^= p[19];
  ret += p[144];
  ret -= p[21];

/* quadratic taps */
  ret -= p[59]&p[289];
  ret += p[253]|p[277];
  ret ^= p[245]&p[195];

  return(ret);
}


local int munge1(register int *p)
{
  register int ret;

  ret = 154605730;

/* linear taps */
  ret ^= p[53];
  ret ^= p[82];
  ret -= p[110];
  ret ^= p[93];
  ret ^= p[139];
  ret += p[31];

/* quadratic taps */
  ret -= p[47]&p[255];
  ret += p[75]&p[97];
  ret += p[183]&p[40];

  return(ret);
}


local int munge2(register int *p)
{
  register int ret;

  ret = 2055183392;

/* linear taps */
  ret -= p[59];
  ret += p[277];
  ret -= p[285];
  ret ^= p[31];
  ret -= p[39];
  ret += p[128];

/* quadratic taps */
  ret -= p[105]|p[225];
  ret ^= p[154]&p[51];
  ret += p[231]|p[229];

  return(ret);
}


local int munge3(register int *p)
{
  register int ret;

  ret = 1148521612;

/* linear taps */
  ret += p[52];
  ret -= p[220];
  ret += p[90];
  ret -= p[86];
  ret ^= p[255];
  ret ^= p[280];

/* quadratic taps */
  ret ^= p[157]&p[186];
  ret -= p[40]&p[150];
  ret ^= p[46]&p[120];

  return(ret);
}


local int munge4(register int *p)
{
  register int ret;

  ret = 494907341;

/* linear taps */
  ret ^= p[42];
  ret += p[57];
  ret ^= p[242];
  ret ^= p[99];
  ret ^= p[78];
  ret += p[109];

/* quadratic taps */
  ret += p[110]|p[51];
  ret ^= p[136]&p[25];
  ret += p[139]&p[145];

  return(ret);
}


local int munge5(register int *p)
{
  register int ret;

  ret = 418748585;

/* linear taps */
  ret ^= p[27];
  ret -= p[72];
  ret ^= p[272];
  ret += p[249];
  ret ^= p[238];
  ret ^= p[94];

/* quadratic taps */
  ret -= p[274]|p[117];
  ret -= p[195]&p[175];
  ret += p[248]&p[180];

  return(ret);
}


local int munge6(register int *p)
{
  register int ret;

  ret = 1640181323;

/* linear taps */
  ret -= p[285];
  ret += p[22];
  ret += p[92];
  ret ^= p[227];
  ret ^= p[52];
  ret ^= p[196];

/* quadratic taps */
  ret ^= p[103]&p[120];
  ret -= p[101]|p[135];
  ret += p[243]&p[207];

  return(ret);
}


local int munge7(register int *p)
{
  register int ret;

  ret = 583760400;

/* linear taps */
  ret -= p[190];
  ret ^= p[120];
  ret ^= p[79];
  ret -= p[159];
  ret ^= p[52];
  ret ^= p[1];

/* quadratic taps */
  ret -= p[193]|p[285];
  ret += p[25]|p[232];
  ret ^= p[291]|p[91];

  return(ret);
}


local int munge8(register int *p)
{
  register int ret;

  ret = -54362599;

/* linear taps */
  ret -= p[231];
  ret += p[225];
  ret ^= p[16];
  ret += p[154];
  ret -= p[27];
  ret += p[130];

/* quadratic taps */
  ret ^= p[289]|p[215];
  ret ^= p[62]&p[3];
  ret ^= p[12]|p[242];

  return(ret);
}


local int munge9(register int *p)
{
  register int ret;

  ret = 442981666;

/* linear taps */
  ret ^= p[220];
  ret ^= p[60];
  ret += p[238];
  ret ^= p[167];
  ret ^= p[186];
  ret -= p[124];

/* quadratic taps */
  ret += p[152]&p[248];
  ret ^= p[36]&p[89];
  ret += p[67]|p[162];

  return(ret);
}


local int munge10(register int *p)
{
  register int ret;

  ret = -312982658;

/* linear taps */
  ret += p[39];
  ret ^= p[177];
  ret -= p[184];
  ret -= p[3];
  ret -= p[102];
  ret ^= p[233];

/* quadratic taps */
  ret += p[218]|p[181];
  ret ^= p[167]|p[113];
  ret += p[48]&p[70];

  return(ret);
}


local int munge11(register int *p)
{
  register int ret;

  ret = 27231582;

/* linear taps */
  ret ^= p[140];
  ret -= p[272];
  ret ^= p[74];
  ret ^= p[108];
  ret += p[119];
  ret -= p[144];

/* quadratic taps */
  ret ^= p[26]|p[224];
  ret ^= p[3]|p[251];
  ret += p[277]|p[60];

  return(ret);
}


local int munge12(register int *p)
{
  register int ret;

  ret = 1529492756;

/* linear taps */
  ret -= p[259];
  ret += p[51];
  ret -= p[103];
  ret += p[152];
  ret ^= p[246];
  ret -= p[47];

/* quadratic taps */
  ret += p[257]|p[222];
  ret ^= p[273]|p[133];
  ret ^= p[233]|p[278];

  return(ret);
}


local int munge13(register int *p)
{
  register int ret;

  ret = 1205213860;

/* linear taps */
  ret ^= p[169];
  ret ^= p[72];
  ret ^= p[116];
  ret ^= p[32];
  ret ^= p[230];
  ret -= p[237];

/* quadratic taps */
  ret -= p[268]|p[17];
  ret ^= p[119]|p[216];
  ret += p[214]&p[95];

  return(ret);
}


local int munge14(register int *p)
{
  register int ret;

  ret = -1962965285;

/* linear taps */
  ret ^= p[52];
  ret -= p[81];
  ret ^= p[255];
  ret -= p[39];
  ret ^= p[247];
  ret ^= p[32];

/* quadratic taps */
  ret ^= p[268]|p[158];
  ret ^= p[99]&p[169];
  ret ^= p[12]&p[272];

  return(ret);
}


local int munge15(register int *p)
{
  register int ret;

  ret = 923756815;

/* linear taps */
  ret += p[184];
  ret ^= p[150];
  ret ^= p[2];
  ret -= p[123];
  ret ^= p[35];
  ret ^= p[26];

/* quadratic taps */
  ret -= p[25]|p[275];
  ret ^= p[6]|p[171];
  ret ^= p[60]&p[139];

  return(ret);
}


local int munge16(register int *p)
{
  register int ret;

  ret = -1014188549;

/* linear taps */
  ret += p[88];
  ret ^= p[230];
  ret ^= p[291];
  ret ^= p[196];
  ret += p[246];
  ret ^= p[97];

/* quadratic taps */
  ret += p[182]&p[115];
  ret += p[142]|p[164];
  ret += p[110]&p[51];

  return(ret);
}


local int munge17(register int *p)
{
  register int ret;

  ret = -1565541715;

/* linear taps */
  ret -= p[204];
  ret -= p[266];
  ret ^= p[70];
  ret ^= p[166];
  ret += p[5];
  ret -= p[89];

/* quadratic taps */
  ret -= p[56]|p[276];
  ret ^= p[273]&p[152];
  ret ^= p[96]|p[45];

  return(ret);
}


local int munge18(register int *p)
{
  register int ret;

  ret = -1206938557;

/* linear taps */
  ret -= p[139];
  ret ^= p[253];
  ret -= p[55];
  ret += p[59];
  ret ^= p[208];
  ret += p[71];

/* quadratic taps */
  ret -= p[142]&p[203];
  ret ^= p[230]&p[213];
  ret ^= p[159]&p[268];

  return(ret);
}


local int munge19(register int *p)
{
  register int ret;

  ret = 1717481475;

/* linear taps */
  ret -= p[189];
  ret ^= p[258];
  ret += p[184];
  ret ^= p[233];
  ret -= p[70];
  ret += p[88];

/* quadratic taps */
  ret -= p[209]&p[144];
  ret -= p[250]&p[131];
  ret -= p[125]&p[270];

  return(ret);
}


local int munge20(register int *p)
{
  register int ret;

  ret = 1496433424;

/* linear taps */
  ret += p[68];
  ret ^= p[203];
  ret -= p[85];
  ret ^= p[30];
  ret += p[205];
  ret ^= p[23];

/* quadratic taps */
  ret -= p[284]&p[167];
  ret ^= p[156]&p[180];
  ret ^= p[24]|p[28];

  return(ret);
}


local int munge21(register int *p)
{
  register int ret;

  ret = -735617802;

/* linear taps */
  ret -= p[153];
  ret += p[289];
  ret ^= p[227];
  ret ^= p[39];
  ret ^= p[112];
  ret += p[182];

/* quadratic taps */
  ret += p[124]|p[292];
  ret ^= p[11]|p[183];
  ret -= p[274]|p[137];

  return(ret);
}


local int munge22(register int *p)
{
  register int ret;

  ret = 1345282939;

/* linear taps */
  ret ^= p[190];
  ret ^= p[250];
  ret += p[239];
  ret += p[194];
  ret += p[258];
  ret += p[175];

/* quadratic taps */
  ret ^= p[90]&p[191];
  ret ^= p[53]&p[129];
  ret -= p[38]|p[277];

  return(ret);
}


local int munge23(register int *p)
{
  register int ret;

  ret = -1881354244;

/* linear taps */
  ret += p[24];
  ret ^= p[283];
  ret -= p[122];
  ret -= p[105];
  ret ^= p[220];
  ret += p[79];

/* quadratic taps */
  ret ^= p[91]|p[98];
  ret += p[138]|p[260];
  ret -= p[106]&p[291];

  return(ret);
}


local int munge24(register int *p)
{
  register int ret;

  ret = -1606827320;

/* linear taps */
  ret ^= p[264];
  ret -= p[19];
  ret -= p[76];
  ret ^= p[158];
  ret ^= p[121];
  ret -= p[278];

/* quadratic taps */
  ret += p[151]|p[186];
  ret += p[97]&p[88];
  ret ^= p[169]&p[214];

  return(ret);
}


local int munge25(register int *p)
{
  register int ret;

  ret = 1643558129;

/* linear taps */
  ret ^= p[109];
  ret ^= p[23];
  ret -= p[276];
  ret ^= p[115];
  ret ^= p[117];
  ret += p[210];

/* quadratic taps */
  ret -= p[253]&p[120];
  ret += p[28]|p[288];
  ret ^= p[64]|p[219];

  return(ret);
}


local int munge26(register int *p)
{
  register int ret;

  ret = 1180983642;

/* linear taps */
  ret -= p[12];
  ret ^= p[205];
  ret ^= p[64];
  ret ^= p[111];
  ret += p[164];
  ret ^= p[244];

/* quadratic taps */
  ret += p[84]|p[2];
  ret ^= p[86]|p[243];
  ret -= p[281]|p[152];

  return(ret);
}


local int munge27(register int *p)
{
  register int ret;

  ret = 330144676;

/* linear taps */
  ret -= p[261];
  ret ^= p[74];
  ret ^= p[99];
  ret ^= p[149];
  ret ^= p[92];
  ret ^= p[291];

/* quadratic taps */
  ret -= p[68]|p[223];
  ret -= p[171]|p[269];
  ret ^= p[24]|p[279];

  return(ret);
}


local int munge28(register int *p)
{
  register int ret;

  ret = 501068918;

/* linear taps */
  ret -= p[75];
  ret += p[206];
  ret ^= p[237];
  ret -= p[154];
  ret ^= p[17];
  ret ^= p[251];

/* quadratic taps */
  ret ^= p[201]|p[271];
  ret -= p[27]|p[290];
  ret += p[55]&p[104];

  return(ret);
}


local int munge29(register int *p)
{
  register int ret;

  ret = -175519215;

/* linear taps */
  ret ^= p[157];
  ret ^= p[184];
  ret -= p[288];
  ret -= p[130];
  ret ^= p[283];
  ret ^= p[224];

/* quadratic taps */
  ret ^= p[160]|p[75];
  ret ^= p[25]|p[58];
  ret += p[114]|p[44];

  return(ret);
}


local int munge30(register int *p)
{
  register int ret;

  ret = 2115076475;

/* linear taps */
  ret += p[264];
  ret += p[182];
  ret -= p[63];
  ret ^= p[78];
  ret ^= p[200];
  ret -= p[5];

/* quadratic taps */
  ret ^= p[206]&p[49];
  ret -= p[240]&p[168];
  ret ^= p[181]|p[46];

  return(ret);
}


local int munge31(register int *p)
{
  register int ret;

  ret = 1515433695;

/* linear taps */
  ret += p[174];
  ret ^= p[12];
  ret += p[17];
  ret ^= p[206];
  ret -= p[86];
  ret -= p[54];

/* quadratic taps */
  ret ^= p[289]|p[270];
  ret ^= p[72]&p[244];
  ret -= p[168]&p[5];

  return(ret);
}


local int munge32(register int *p)
{
  register int ret;

  ret = 219402494;

/* linear taps */
  ret += p[41];
  ret ^= p[169];
  ret ^= p[87];
  ret -= p[187];
  ret ^= p[48];
  ret += p[219];

/* quadratic taps */
  ret ^= p[106]&p[157];
  ret ^= p[118]&p[224];
  ret -= p[225]&p[134];

  return(ret);
}


local int munge33(register int *p)
{
  register int ret;

  ret = -1925992736;

/* linear taps */
  ret += p[13];
  ret ^= p[34];
  ret ^= p[205];
  ret ^= p[283];
  ret -= p[131];
  ret += p[188];

/* quadratic taps */
  ret ^= p[125]&p[87];
  ret -= p[215]&p[1];
  ret -= p[16]|p[6];

  return(ret);
}


local int munge34(register int *p)
{
  register int ret;

  ret = -1213864246;

/* linear taps */
  ret ^= p[55];
  ret ^= p[118];
  ret -= p[182];
  ret ^= p[10];
  ret ^= p[130];
  ret ^= p[75];

/* quadratic taps */
  ret += p[65]|p[177];
  ret ^= p[148]|p[160];
  ret ^= p[279]|p[155];

  return(ret);
}


local int munge35(register int *p)
{
  register int ret;

  ret = 1169395130;

/* linear taps */
  ret -= p[123];
  ret ^= p[172];
  ret ^= p[171];
  ret += p[63];
  ret += p[108];
  ret -= p[113];

/* quadratic taps */
  ret -= p[246]&p[164];
  ret ^= p[214]|p[189];
  ret ^= p[242]&p[255];

  return(ret);
}


local int munge36(register int *p)
{
  register int ret;

  ret = 1899457123;

/* linear taps */
  ret ^= p[94];
  ret ^= p[235];
  ret -= p[242];
  ret ^= p[44];
  ret ^= p[3];
  ret ^= p[23];

/* quadratic taps */
  ret ^= p[93]|p[216];
  ret -= p[212]|p[236];
  ret ^= p[292]|p[271];

  return(ret);
}


local int munge37(register int *p)
{
  register int ret;

  ret = 44578854;

/* linear taps */
  ret ^= p[284];
  ret ^= p[31];
  ret -= p[56];
  ret -= p[126];
  ret -= p[225];
  ret ^= p[246];

/* quadratic taps */
  ret ^= p[159]|p[274];
  ret -= p[283]|p[7];
  ret -= p[166]&p[131];

  return(ret);
}


local int munge38(register int *p)
{
  register int ret;

  ret = -1920991715;

/* linear taps */
  ret ^= p[281];
  ret ^= p[145];
  ret ^= p[146];
  ret -= p[85];
  ret ^= p[284];
  ret ^= p[84];

/* quadratic taps */
  ret -= p[265]&p[132];
  ret += p[220]|p[26];
  ret -= p[277]&p[70];

  return(ret);
}


local int munge39(register int *p)
{
  register int ret;

  ret = -1971321148;

/* linear taps */
  ret -= p[228];
  ret -= p[205];
  ret ^= p[127];
  ret ^= p[132];
  ret ^= p[119];
  ret ^= p[278];

/* quadratic taps */
  ret ^= p[199]|p[230];
  ret ^= p[63]&p[96];
  ret -= p[117]|p[80];

  return(ret);
}


local int munge40(register int *p)
{
  register int ret;

  ret = 873595319;

/* linear taps */
  ret += p[192];
  ret ^= p[253];
  ret ^= p[205];
  ret ^= p[114];
  ret -= p[4];
  ret ^= p[167];

/* quadratic taps */
  ret ^= p[217]|p[96];
  ret += p[213]&p[109];
  ret ^= p[72]|p[55];

  return(ret);
}


local int munge41(register int *p)
{
  register int ret;

  ret = -1603648999;

/* linear taps */
  ret ^= p[70];
  ret += p[184];
  ret += p[203];
  ret ^= p[66];
  ret ^= p[74];
  ret -= p[68];

/* quadratic taps */
  ret -= p[263]&p[22];
  ret += p[46]|p[231];
  ret ^= p[108]&p[54];

  return(ret);
}


local int munge42(register int *p)
{
  register int ret;

  ret = 598154211;

/* linear taps */
  ret ^= p[258];
  ret ^= p[219];
  ret -= p[122];
  ret -= p[70];
  ret ^= p[288];
  ret ^= p[231];

/* quadratic taps */
  ret -= p[158]&p[263];
  ret ^= p[173]|p[242];
  ret ^= p[5]&p[257];

  return(ret);
}


local int munge43(register int *p)
{
  register int ret;

  ret = 229176167;

/* linear taps */
  ret -= p[255];
  ret += p[28];
  ret ^= p[230];
  ret ^= p[93];
  ret -= p[60];
  ret -= p[279];

/* quadratic taps */
  ret += p[2]&p[138];
  ret += p[45]&p[218];
  ret ^= p[197]|p[272];

  return(ret);
}


local int munge44(register int *p)
{
  register int ret;

  ret = 2103084522;

/* linear taps */
  ret -= p[116];
  ret -= p[189];
  ret += p[133];
  ret -= p[134];
  ret += p[146];
  ret += p[280];

/* quadratic taps */
  ret += p[241]|p[104];
  ret += p[171]&p[93];
  ret ^= p[240]&p[52];

  return(ret);
}


local int munge45(register int *p)
{
  register int ret;

  ret = -2030217316;

/* linear taps */
  ret += p[253];
  ret ^= p[28];
  ret += p[216];
  ret += p[60];
  ret ^= p[16];
  ret ^= p[271];

/* quadratic taps */
  ret ^= p[186]|p[176];
  ret += p[30]|p[116];
  ret ^= p[214]&p[72];

  return(ret);
}


local int munge46(register int *p)
{
  register int ret;

  ret = 1878061094;

/* linear taps */
  ret -= p[151];
  ret ^= p[5];
  ret ^= p[284];
  ret ^= p[164];
  ret += p[2];
  ret += p[111];

/* quadratic taps */
  ret -= p[15]|p[158];
  ret += p[4]|p[107];
  ret += p[200]|p[215];

  return(ret);
}


local int munge47(register int *p)
{
  register int ret;

  ret = -2004308555;

/* linear taps */
  ret -= p[118];
  ret ^= p[190];
  ret -= p[170];
  ret += p[267];
  ret += p[102];
  ret ^= p[132];

/* quadratic taps */
  ret -= p[80]&p[159];
  ret ^= p[191]|p[57];
  ret ^= p[175]&p[14];

  return(ret);
}


local int munge48(register int *p)
{
  register int ret;

  ret = -986387168;

/* linear taps */
  ret ^= p[17];
  ret += p[214];
  ret ^= p[97];
  ret ^= p[210];
  ret += p[286];
  ret ^= p[82];

/* quadratic taps */
  ret += p[143]|p[162];
  ret += p[111]|p[281];
  ret ^= p[259]|p[20];

  return(ret);
}


local int munge49(register int *p)
{
  register int ret;

  ret = 146132206;

/* linear taps */
  ret ^= p[158];
  ret += p[57];
  ret += p[145];
  ret -= p[220];
  ret ^= p[138];
  ret ^= p[122];

/* quadratic taps */
  ret ^= p[277]|p[235];
  ret += p[76]|p[82];
  ret ^= p[97]|p[88];

  return(ret);
}


local int munge50(register int *p)
{
  register int ret;

  ret = 476174802;

/* linear taps */
  ret -= p[16];
  ret += p[94];
  ret -= p[105];
  ret += p[217];
  ret ^= p[59];
  ret ^= p[168];

/* quadratic taps */
  ret -= p[218]|p[130];
  ret -= p[292]&p[246];
  ret ^= p[111]&p[8];

  return(ret);
}


local int munge51(register int *p)
{
  register int ret;

  ret = -1813062979;

/* linear taps */
  ret -= p[204];
  ret += p[75];
  ret ^= p[241];
  ret ^= p[169];
  ret -= p[87];
  ret ^= p[158];

/* quadratic taps */
  ret -= p[144]|p[186];
  ret ^= p[77]|p[59];
  ret ^= p[222]|p[255];

  return(ret);
}


local int munge52(register int *p)
{
  register int ret;

  ret = -956034496;

/* linear taps */
  ret -= p[163];
  ret += p[113];
  ret ^= p[46];
  ret ^= p[143];
  ret += p[52];
  ret += p[126];

/* quadratic taps */
  ret ^= p[257]|p[150];
  ret -= p[180]|p[85];
  ret ^= p[57]&p[224];

  return(ret);
}


local int munge53(register int *p)
{
  register int ret;

  ret = -1941828400;

/* linear taps */
  ret -= p[124];
  ret += p[144];
  ret ^= p[247];
  ret -= p[165];
  ret ^= p[110];
  ret += p[216];

/* quadratic taps */
  ret ^= p[90]&p[292];
  ret -= p[245]&p[122];
  ret += p[27]|p[6];

  return(ret);
}


local int munge54(register int *p)
{
  register int ret;

  ret = -890968885;

/* linear taps */
  ret ^= p[227];
  ret -= p[272];
  ret ^= p[198];
  ret -= p[286];
  ret += p[74];
  ret ^= p[42];

/* quadratic taps */
  ret ^= p[117]&p[31];
  ret ^= p[280]&p[69];
  ret ^= p[241]&p[195];

  return(ret);
}


local int munge55(register int *p)
{
  register int ret;

  ret = -199674425;

/* linear taps */
  ret ^= p[284];
  ret ^= p[82];
  ret -= p[118];
  ret ^= p[68];
  ret ^= p[10];
  ret ^= p[248];

/* quadratic taps */
  ret += p[124]|p[79];
  ret -= p[4]|p[265];
  ret += p[227]|p[93];

  return(ret);
}


local int munge56(register int *p)
{
  register int ret;

  ret = 304915956;

/* linear taps */
  ret += p[104];
  ret += p[237];
  ret ^= p[164];
  ret ^= p[40];
  ret -= p[133];
  ret ^= p[33];

/* quadratic taps */
  ret ^= p[265]&p[122];
  ret += p[99]|p[146];
  ret -= p[25]&p[21];

  return(ret);
}


local int munge57(register int *p)
{
  register int ret;

  ret = -1287679661;

/* linear taps */
  ret ^= p[76];
  ret += p[92];
  ret ^= p[203];
  ret ^= p[93];
  ret -= p[28];
  ret -= p[288];

/* quadratic taps */
  ret -= p[188]&p[124];
  ret += p[271]|p[103];
  ret += p[229]&p[180];

  return(ret);
}


local int munge58(register int *p)
{
  register int ret;

  ret = 1115177860;

/* linear taps */
  ret ^= p[88];
  ret ^= p[278];
  ret += p[28];
  ret ^= p[207];
  ret -= p[137];
  ret -= p[136];

/* quadratic taps */
  ret += p[264]|p[2];
  ret ^= p[84]&p[119];
  ret ^= p[217]&p[33];

  return(ret);
}


local int munge59(register int *p)
{
  register int ret;

  ret = 479342223;

/* linear taps */
  ret += p[215];
  ret ^= p[50];
  ret ^= p[207];
  ret ^= p[129];
  ret ^= p[226];
  ret -= p[235];

/* quadratic taps */
  ret += p[218]&p[1];
  ret ^= p[109]|p[245];
  ret -= p[288]|p[134];

  return(ret);
}


local int munge60(register int *p)
{
  register int ret;

  ret = -1362311168;

/* linear taps */
  ret += p[289];
  ret ^= p[257];
  ret += p[37];
  ret += p[1];
  ret ^= p[152];
  ret += p[9];

/* quadratic taps */
  ret -= p[94]&p[279];
  ret -= p[203]|p[57];
  ret += p[202]|p[248];

  return(ret);
}


local int munge61(register int *p)
{
  register int ret;

  ret = -1585949487;

/* linear taps */
  ret += p[170];
  ret ^= p[79];
  ret ^= p[32];
  ret ^= p[236];
  ret ^= p[148];
  ret ^= p[288];

/* quadratic taps */
  ret ^= p[44]|p[52];
  ret ^= p[10]|p[80];
  ret ^= p[110]|p[198];

  return(ret);
}


local int munge62(register int *p)
{
  register int ret;

  ret = 89035250;

/* linear taps */
  ret ^= p[142];
  ret ^= p[8];
  ret -= p[233];
  ret -= p[285];
  ret += p[211];
  ret ^= p[113];

/* quadratic taps */
  ret += p[263]&p[272];
  ret -= p[148]&p[249];
  ret -= p[16]|p[161];

  return(ret);
}


local int munge63(register int *p)
{
  register int ret;

  ret = 50172021;

/* linear taps */
  ret -= p[111];
  ret ^= p[195];
  ret -= p[288];
  ret ^= p[208];
  ret ^= p[66];
  ret += p[61];

/* quadratic taps */
  ret ^= p[180]|p[286];
  ret ^= p[157]&p[244];
  ret -= p[128]&p[22];

  return(ret);
}


local int munge64(register int *p)
{
  register int ret;

  ret = -589083951;

/* linear taps */
  ret ^= p[104];
  ret ^= p[30];
  ret ^= p[184];
  ret += p[232];
  ret += p[159];
  ret ^= p[20];

/* quadratic taps */
  ret += p[112]|p[53];
  ret ^= p[204]&p[241];
  ret ^= p[163]|p[291];

  return(ret);
}


local int munge65(register int *p)
{
  register int ret;

  ret = 1108012973;

/* linear taps */
  ret ^= p[285];
  ret -= p[139];
  ret -= p[147];
  ret += p[271];
  ret -= p[39];
  ret += p[291];

/* quadratic taps */
  ret += p[244]|p[250];
  ret ^= p[110]|p[129];
  ret -= p[223]|p[245];

  return(ret);
}


local int munge66(register int *p)
{
  register int ret;

  ret = 1747229533;

/* linear taps */
  ret += p[83];
  ret ^= p[103];
  ret ^= p[265];
  ret ^= p[171];
  ret -= p[7];
  ret -= p[226];

/* quadratic taps */
  ret -= p[287]&p[242];
  ret ^= p[275]|p[94];
  ret -= p[111]|p[283];

  return(ret);
}


local int munge67(register int *p)
{
  register int ret;

  ret = 2028348062;

/* linear taps */
  ret ^= p[168];
  ret += p[154];
  ret -= p[62];
  ret += p[28];
  ret ^= p[1];
  ret ^= p[85];

/* quadratic taps */
  ret ^= p[283]|p[241];
  ret += p[159]|p[285];
  ret += p[63]&p[34];

  return(ret);
}


local int munge68(register int *p)
{
  register int ret;

  ret = -1088662824;

/* linear taps */
  ret ^= p[186];
  ret ^= p[214];
  ret ^= p[104];
  ret ^= p[230];
  ret += p[132];
  ret ^= p[20];

/* quadratic taps */
  ret += p[123]|p[190];
  ret -= p[256]|p[169];
  ret += p[90]|p[134];

  return(ret);
}


local int munge69(register int *p)
{
  register int ret;

  ret = -2047027474;

/* linear taps */
  ret -= p[178];
  ret ^= p[222];
  ret ^= p[58];
  ret ^= p[181];
  ret ^= p[246];
  ret -= p[284];

/* quadratic taps */
  ret += p[48]&p[205];
  ret -= p[200]&p[249];
  ret ^= p[263]|p[169];

  return(ret);
}


local int munge70(register int *p)
{
  register int ret;

  ret = 322113791;

/* linear taps */
  ret ^= p[35];
  ret ^= p[127];
  ret ^= p[201];
  ret -= p[263];
  ret += p[179];
  ret += p[34];

/* quadratic taps */
  ret ^= p[186]|p[260];
  ret ^= p[209]|p[46];
  ret -= p[115]&p[154];

  return(ret);
}


local int munge71(register int *p)
{
  register int ret;

  ret = -1966830429;

/* linear taps */
  ret -= p[27];
  ret += p[76];
  ret -= p[38];
  ret ^= p[25];
  ret += p[209];
  ret += p[198];

/* quadratic taps */
  ret ^= p[161]&p[144];
  ret ^= p[205]&p[125];
  ret ^= p[252]&p[97];

  return(ret);
}


local int munge72(register int *p)
{
  register int ret;

  ret = -1888069517;

/* linear taps */
  ret ^= p[148];
  ret ^= p[208];
  ret ^= p[245];
  ret -= p[5];
  ret ^= p[41];
  ret ^= p[86];

/* quadratic taps */
  ret += p[200]|p[235];
  ret ^= p[50]|p[97];
  ret ^= p[150]&p[16];

  return(ret);
}


local int munge73(register int *p)
{
  register int ret;

  ret = -988543966;

/* linear taps */
  ret -= p[151];
  ret += p[227];
  ret ^= p[215];
  ret += p[255];
  ret ^= p[167];
  ret ^= p[279];

/* quadratic taps */
  ret ^= p[62]|p[5];
  ret -= p[46]|p[130];
  ret -= p[83]|p[182];

  return(ret);
}


local int munge74(register int *p)
{
  register int ret;

  ret = 1266474651;

/* linear taps */
  ret ^= p[112];
  ret += p[191];
  ret ^= p[159];
  ret ^= p[217];
  ret -= p[254];
  ret -= p[251];

/* quadratic taps */
  ret ^= p[84]&p[286];
  ret -= p[23]|p[164];
  ret += p[134]|p[199];

  return(ret);
}


local int munge75(register int *p)
{
  register int ret;

  ret = -1070907754;

/* linear taps */
  ret += p[76];
  ret += p[95];
  ret += p[20];
  ret += p[238];
  ret += p[119];
  ret += p[205];

/* quadratic taps */
  ret ^= p[102]&p[209];
  ret += p[130]|p[244];
  ret -= p[126]|p[103];

  return(ret);
}


local int munge76(register int *p)
{
  register int ret;

  ret = 1153166174;

/* linear taps */
  ret ^= p[238];
  ret ^= p[278];
  ret ^= p[24];
  ret ^= p[73];
  ret ^= p[89];
  ret -= p[161];

/* quadratic taps */
  ret ^= p[171]&p[225];
  ret ^= p[217]&p[142];
  ret -= p[28]|p[75];

  return(ret);
}


local int munge77(register int *p)
{
  register int ret;

  ret = -1514278163;

/* linear taps */
  ret ^= p[54];
  ret ^= p[154];
  ret ^= p[166];
  ret ^= p[178];
  ret += p[64];
  ret ^= p[22];

/* quadratic taps */
  ret ^= p[261]&p[224];
  ret ^= p[190]|p[98];
  ret ^= p[276]|p[290];

  return(ret);
}


local int munge78(register int *p)
{
  register int ret;

  ret = 1184232337;

/* linear taps */
  ret ^= p[9];
  ret -= p[141];
  ret ^= p[20];
  ret ^= p[54];
  ret ^= p[199];
  ret += p[248];

/* quadratic taps */
  ret -= p[210]|p[65];
  ret ^= p[145]|p[171];
  ret -= p[218]|p[173];

  return(ret);
}


local int munge79(register int *p)
{
  register int ret;

  ret = -1871487669;

/* linear taps */
  ret ^= p[222];
  ret ^= p[161];
  ret += p[231];
  ret ^= p[137];
  ret -= p[82];
  ret += p[280];

/* quadratic taps */
  ret ^= p[87]|p[51];
  ret -= p[75]&p[86];
  ret ^= p[53]&p[130];

  return(ret);
}


local int munge80(register int *p)
{
  register int ret;

  ret = -770868268;

/* linear taps */
  ret ^= p[245];
  ret ^= p[140];
  ret ^= p[126];
  ret += p[173];
  ret ^= p[139];
  ret -= p[204];

/* quadratic taps */
  ret -= p[158]|p[42];
  ret ^= p[161]|p[188];
  ret += p[189]|p[92];

  return(ret);
}


local int munge81(register int *p)
{
  register int ret;

  ret = 2135971519;

/* linear taps */
  ret ^= p[96];
  ret ^= p[120];
  ret ^= p[84];
  ret -= p[176];
  ret ^= p[255];
  ret -= p[256];

/* quadratic taps */
  ret += p[221]&p[222];
  ret ^= p[29]&p[89];
  ret ^= p[286]|p[46];

  return(ret);
}


local int munge82(register int *p)
{
  register int ret;

  ret = 1452485291;

/* linear taps */
  ret ^= p[37];
  ret -= p[153];
  ret ^= p[105];
  ret ^= p[31];
  ret ^= p[273];
  ret += p[42];

/* quadratic taps */
  ret ^= p[100]|p[224];
  ret += p[33]&p[194];
  ret += p[115]|p[102];

  return(ret);
}


local int munge83(register int *p)
{
  register int ret;

  ret = 2033907243;

/* linear taps */
  ret -= p[184];
  ret -= p[65];
  ret += p[147];
  ret ^= p[26];
  ret += p[176];
  ret += p[52];

/* quadratic taps */
  ret ^= p[121]|p[27];
  ret ^= p[12]&p[164];
  ret ^= p[144]&p[232];

  return(ret);
}


local int munge84(register int *p)
{
  register int ret;

  ret = -30054111;

/* linear taps */
  ret -= p[39];
  ret -= p[46];
  ret -= p[135];
  ret += p[29];
  ret += p[124];
  ret -= p[270];

/* quadratic taps */
  ret ^= p[143]&p[189];
  ret ^= p[195]&p[12];
  ret -= p[196]|p[133];

  return(ret);
}


local int munge85(register int *p)
{
  register int ret;

  ret = 1491130031;

/* linear taps */
  ret -= p[75];
  ret += p[137];
  ret += p[269];
  ret ^= p[238];
  ret += p[261];
  ret ^= p[200];

/* quadratic taps */
  ret -= p[150]&p[74];
  ret -= p[146]|p[131];
  ret -= p[32]&p[11];

  return(ret);
}


local int munge86(register int *p)
{
  register int ret;

  ret = -1551915598;

/* linear taps */
  ret -= p[61];
  ret ^= p[135];
  ret -= p[222];
  ret -= p[45];
  ret ^= p[190];
  ret -= p[118];

/* quadratic taps */
  ret -= p[145]|p[173];
  ret ^= p[234]|p[210];
  ret -= p[270]&p[106];

  return(ret);
}


local int munge87(register int *p)
{
  register int ret;

  ret = 889527709;

/* linear taps */
  ret -= p[194];
  ret ^= p[44];
  ret -= p[99];
  ret ^= p[127];
  ret += p[16];
  ret -= p[243];

/* quadratic taps */
  ret ^= p[26]&p[33];
  ret += p[157]|p[91];
  ret -= p[10]|p[191];

  return(ret);
}


local int munge88(register int *p)
{
  register int ret;

  ret = -1515365511;

/* linear taps */
  ret -= p[4];
  ret -= p[212];
  ret += p[241];
  ret -= p[158];
  ret -= p[135];
  ret ^= p[175];

/* quadratic taps */
  ret -= p[289]&p[174];
  ret -= p[79]|p[219];
  ret ^= p[57]|p[15];

  return(ret);
}


local int munge89(register int *p)
{
  register int ret;

  ret = -417578172;

/* linear taps */
  ret ^= p[7];
  ret ^= p[184];
  ret ^= p[27];
  ret += p[180];
  ret ^= p[57];
  ret -= p[34];

/* quadratic taps */
  ret += p[236]|p[167];
  ret ^= p[169]&p[282];
  ret += p[273]|p[152];

  return(ret);
}


local int munge90(register int *p)
{
  register int ret;

  ret = 1893380209;

/* linear taps */
  ret -= p[46];
  ret ^= p[63];
  ret ^= p[19];
  ret += p[1];
  ret -= p[80];
  ret += p[18];

/* quadratic taps */
  ret += p[264]&p[144];
  ret += p[112]&p[9];
  ret ^= p[153]|p[221];

  return(ret);
}


local int munge91(register int *p)
{
  register int ret;

  ret = -1288446846;

/* linear taps */
  ret += p[150];
  ret ^= p[14];
  ret ^= p[144];
  ret ^= p[166];
  ret += p[220];
  ret ^= p[50];

/* quadratic taps */
  ret ^= p[276]|p[187];
  ret -= p[285]&p[37];
  ret ^= p[234]|p[48];

  return(ret);
}


local int munge92(register int *p)
{
  register int ret;

  ret = 1387500268;

/* linear taps */
  ret ^= p[279];
  ret -= p[118];
  ret ^= p[152];
  ret -= p[278];
  ret += p[147];
  ret ^= p[168];

/* quadratic taps */
  ret ^= p[26]|p[38];
  ret -= p[140]|p[81];
  ret -= p[251]|p[181];

  return(ret);
}


local int munge93(register int *p)
{
  register int ret;

  ret = -391076543;

/* linear taps */
  ret -= p[64];
  ret ^= p[47];
  ret += p[129];
  ret -= p[89];
  ret ^= p[38];
  ret ^= p[25];

/* quadratic taps */
  ret += p[286]&p[128];
  ret ^= p[94]&p[101];
  ret += p[57]|p[220];

  return(ret);
}


local int munge94(register int *p)
{
  register int ret;

  ret = -934504356;

/* linear taps */
  ret += p[222];
  ret ^= p[123];
  ret -= p[71];
  ret -= p[243];
  ret ^= p[168];
  ret ^= p[212];

/* quadratic taps */
  ret ^= p[268]&p[152];
  ret ^= p[173]&p[17];
  ret -= p[90]&p[167];

  return(ret);
}


local int munge95(register int *p)
{
  register int ret;

  ret = 1209700874;

/* linear taps */
  ret ^= p[103];
  ret += p[23];
  ret ^= p[147];
  ret -= p[11];
  ret ^= p[217];
  ret ^= p[238];

/* quadratic taps */
  ret -= p[99]&p[100];
  ret ^= p[13]|p[37];
  ret ^= p[4]&p[85];

  return(ret);
}


local int munge96(register int *p)
{
  register int ret;

  ret = 626415641;

/* linear taps */
  ret += p[62];
  ret += p[186];
  ret += p[39];
  ret ^= p[146];
  ret += p[154];
  ret ^= p[248];

/* quadratic taps */
  ret ^= p[279]|p[29];
  ret += p[88]|p[283];
  ret ^= p[229]|p[63];

  return(ret);
}


local int munge97(register int *p)
{
  register int ret;

  ret = -121561685;

/* linear taps */
  ret -= p[240];
  ret += p[48];
  ret ^= p[174];
  ret += p[252];
  ret -= p[122];
  ret ^= p[162];

/* quadratic taps */
  ret -= p[44]&p[285];
  ret -= p[84]|p[11];
  ret -= p[292]&p[220];

  return(ret);
}


local int munge98(register int *p)
{
  register int ret;

  ret = 225499947;

/* linear taps */
  ret ^= p[197];
  ret ^= p[92];
  ret += p[35];
  ret += p[279];
  ret ^= p[46];
  ret ^= p[288];

/* quadratic taps */
  ret += p[256]|p[73];
  ret -= p[233]&p[137];
  ret ^= p[106]&p[169];

  return(ret);
}


local int munge99(register int *p)
{
  register int ret;

  ret = -116935755;

/* linear taps */
  ret -= p[125];
  ret ^= p[76];
  ret -= p[89];
  ret += p[228];
  ret ^= p[59];
  ret ^= p[174];

/* quadratic taps */
  ret += p[211]&p[81];
  ret -= p[273]|p[66];
  ret ^= p[275]&p[254];

  return(ret);
}


local int munge100(register int *p)
{
  register int ret;

  ret = -385032313;

/* linear taps */
  ret ^= p[127];
  ret ^= p[177];
  ret -= p[208];
  ret += p[185];
  ret ^= p[133];
  ret ^= p[12];

/* quadratic taps */
  ret ^= p[210]|p[235];
  ret ^= p[49]&p[231];
  ret -= p[227]|p[199];

  return(ret);
}


local int munge101(register int *p)
{
  register int ret;

  ret = -678528750;

/* linear taps */
  ret -= p[43];
  ret ^= p[225];
  ret -= p[97];
  ret += p[190];
  ret -= p[60];
  ret ^= p[200];

/* quadratic taps */
  ret += p[236]|p[31];
  ret ^= p[11]&p[280];
  ret += p[263]&p[216];

  return(ret);
}


local int munge102(register int *p)
{
  register int ret;

  ret = 91257425;

/* linear taps */
  ret += p[267];
  ret ^= p[219];
  ret ^= p[266];
  ret -= p[16];
  ret += p[252];
  ret += p[277];

/* quadratic taps */
  ret -= p[143]|p[9];
  ret += p[5]&p[125];
  ret ^= p[189]|p[172];

  return(ret);
}


local int munge103(register int *p)
{
  register int ret;

  ret = 819983029;

/* linear taps */
  ret ^= p[30];
  ret ^= p[288];
  ret += p[31];
  ret += p[192];
  ret += p[38];
  ret ^= p[108];

/* quadratic taps */
  ret ^= p[253]|p[254];
  ret += p[41]|p[244];
  ret -= p[163]&p[206];

  return(ret);
}


local int munge104(register int *p)
{
  register int ret;

  ret = -643319513;

/* linear taps */
  ret += p[227];
  ret ^= p[253];
  ret += p[220];
  ret -= p[18];
  ret ^= p[255];
  ret ^= p[136];

/* quadratic taps */
  ret ^= p[138]|p[51];
  ret += p[108]&p[146];
  ret ^= p[26]&p[242];

  return(ret);
}


local int munge105(register int *p)
{
  register int ret;

  ret = -1025330748;

/* linear taps */
  ret += p[96];
  ret ^= p[62];
  ret -= p[115];
  ret ^= p[261];
  ret ^= p[173];
  ret ^= p[234];

/* quadratic taps */
  ret -= p[239]|p[25];
  ret ^= p[281]|p[271];
  ret ^= p[28]|p[109];

  return(ret);
}


local int munge106(register int *p)
{
  register int ret;

  ret = 1040840558;

/* linear taps */
  ret ^= p[67];
  ret += p[273];
  ret ^= p[283];
  ret ^= p[190];
  ret ^= p[194];
  ret ^= p[171];

/* quadratic taps */
  ret ^= p[78]|p[77];
  ret += p[129]&p[65];
  ret ^= p[70]|p[83];

  return(ret);
}


local int munge107(register int *p)
{
  register int ret;

  ret = 1407314002;

/* linear taps */
  ret ^= p[173];
  ret += p[73];
  ret ^= p[37];
  ret += p[47];
  ret ^= p[77];
  ret ^= p[70];

/* quadratic taps */
  ret ^= p[167]|p[193];
  ret += p[84]&p[74];
  ret += p[10]|p[71];

  return(ret);
}


local int munge108(register int *p)
{
  register int ret;

  ret = 1135564277;

/* linear taps */
  ret ^= p[165];
  ret -= p[110];
  ret -= p[237];
  ret ^= p[158];
  ret += p[138];
  ret ^= p[182];

/* quadratic taps */
  ret ^= p[286]&p[222];
  ret += p[249]&p[104];
  ret -= p[247]|p[7];

  return(ret);
}


local int munge109(register int *p)
{
  register int ret;

  ret = -2060539408;

/* linear taps */
  ret ^= p[175];
  ret += p[118];
  ret ^= p[2];
  ret ^= p[170];
  ret ^= p[160];
  ret ^= p[147];

/* quadratic taps */
  ret ^= p[95]&p[69];
  ret ^= p[60]&p[179];
  ret += p[86]|p[142];

  return(ret);
}


local int munge110(register int *p)
{
  register int ret;

  ret = 647924821;

/* linear taps */
  ret ^= p[134];
  ret ^= p[76];
  ret -= p[188];
  ret ^= p[219];
  ret -= p[171];
  ret ^= p[232];

/* quadratic taps */
  ret -= p[111]|p[165];
  ret ^= p[197]&p[77];
  ret ^= p[123]|p[255];

  return(ret);
}


local int munge111(register int *p)
{
  register int ret;

  ret = -763379346;

/* linear taps */
  ret ^= p[33];
  ret += p[285];
  ret -= p[289];
  ret ^= p[67];
  ret -= p[279];
  ret ^= p[181];

/* quadratic taps */
  ret += p[110]|p[220];
  ret ^= p[240]|p[15];
  ret ^= p[290]&p[186];

  return(ret);
}


local int munge112(register int *p)
{
  register int ret;

  ret = 1562381723;

/* linear taps */
  ret ^= p[227];
  ret -= p[14];
  ret ^= p[184];
  ret += p[138];
  ret ^= p[203];
  ret ^= p[215];

/* quadratic taps */
  ret -= p[61]&p[95];
  ret += p[221]|p[163];
  ret += p[27]|p[38];

  return(ret);
}


local int munge113(register int *p)
{
  register int ret;

  ret = 394411775;

/* linear taps */
  ret -= p[108];
  ret -= p[217];
  ret += p[212];
  ret += p[270];
  ret -= p[255];
  ret += p[172];

/* quadratic taps */
  ret ^= p[259]|p[175];
  ret ^= p[204]|p[253];
  ret -= p[243]&p[84];

  return(ret);
}


local int munge114(register int *p)
{
  register int ret;

  ret = 1797870428;

/* linear taps */
  ret += p[188];
  ret -= p[163];
  ret -= p[13];
  ret -= p[227];
  ret -= p[274];
  ret += p[99];

/* quadratic taps */
  ret -= p[6]&p[85];
  ret -= p[42]|p[197];
  ret += p[250]&p[251];

  return(ret);
}


local int munge115(register int *p)
{
  register int ret;

  ret = -1463265013;

/* linear taps */
  ret += p[169];
  ret ^= p[19];
  ret ^= p[94];
  ret ^= p[68];
  ret ^= p[188];
  ret += p[239];

/* quadratic taps */
  ret += p[174]|p[79];
  ret -= p[67]&p[199];
  ret ^= p[54]&p[207];

  return(ret);
}


local int munge116(register int *p)
{
  register int ret;

  ret = -170451885;

/* linear taps */
  ret ^= p[133];
  ret += p[285];
  ret -= p[74];
  ret ^= p[180];
  ret += p[134];
  ret -= p[195];

/* quadratic taps */
  ret -= p[3]|p[54];
  ret ^= p[110]|p[280];
  ret += p[39]&p[34];

  return(ret);
}


local int munge117(register int *p)
{
  register int ret;

  ret = -228275702;

/* linear taps */
  ret -= p[116];
  ret ^= p[291];
  ret ^= p[33];
  ret += p[28];
  ret ^= p[85];
  ret ^= p[270];

/* quadratic taps */
  ret += p[91]|p[157];
  ret ^= p[95]|p[264];
  ret -= p[217]&p[134];

  return(ret);
}


local int munge118(register int *p)
{
  register int ret;

  ret = -366423236;

/* linear taps */
  ret -= p[32];
  ret ^= p[11];
  ret ^= p[160];
  ret ^= p[66];
  ret ^= p[282];
  ret -= p[61];

/* quadratic taps */
  ret += p[235]|p[251];
  ret += p[153]&p[79];
  ret ^= p[26]&p[238];

  return(ret);
}


local int munge119(register int *p)
{
  register int ret;

  ret = -1451441225;

/* linear taps */
  ret ^= p[81];
  ret ^= p[240];
  ret -= p[210];
  ret += p[59];
  ret ^= p[180];
  ret += p[35];

/* quadratic taps */
  ret ^= p[273]|p[175];
  ret ^= p[57]&p[23];
  ret ^= p[275]|p[166];

  return(ret);
}


local int munge120(register int *p)
{
  register int ret;

  ret = 1167128177;

/* linear taps */
  ret -= p[185];
  ret += p[173];
  ret ^= p[186];
  ret ^= p[220];
  ret -= p[155];
  ret ^= p[183];

/* quadratic taps */
  ret -= p[150]|p[224];
  ret ^= p[171]&p[230];
  ret ^= p[117]&p[146];

  return(ret);
}


local int munge121(register int *p)
{
  register int ret;

  ret = -523459073;

/* linear taps */
  ret ^= p[271];
  ret ^= p[202];
  ret ^= p[150];
  ret += p[10];
  ret ^= p[182];
  ret += p[107];

/* quadratic taps */
  ret -= p[169]|p[71];
  ret ^= p[31]|p[68];
  ret ^= p[140]&p[132];

  return(ret);
}


local int munge122(register int *p)
{
  register int ret;

  ret = 639445189;

/* linear taps */
  ret ^= p[49];
  ret ^= p[258];
  ret -= p[84];
  ret ^= p[188];
  ret += p[151];
  ret -= p[37];

/* quadratic taps */
  ret ^= p[32]&p[191];
  ret += p[17]|p[209];
  ret -= p[139]&p[25];

  return(ret);
}


local int munge123(register int *p)
{
  register int ret;

  ret = -312735070;

/* linear taps */
  ret ^= p[57];
  ret ^= p[155];
  ret ^= p[26];
  ret ^= p[288];
  ret ^= p[132];
  ret += p[136];

/* quadratic taps */
  ret ^= p[233]&p[247];
  ret ^= p[242]|p[238];
  ret += p[120]|p[253];

  return(ret);
}


local int munge124(register int *p)
{
  register int ret;

  ret = -93913754;

/* linear taps */
  ret -= p[100];
  ret += p[3];
  ret ^= p[13];
  ret -= p[275];
  ret -= p[267];
  ret ^= p[223];

/* quadratic taps */
  ret ^= p[265]|p[22];
  ret ^= p[46]&p[175];
  ret ^= p[215]|p[266];

  return(ret);
}


local int munge125(register int *p)
{
  register int ret;

  ret = -200049632;

/* linear taps */
  ret -= p[217];
  ret -= p[291];
  ret -= p[8];
  ret += p[93];
  ret += p[233];
  ret -= p[96];

/* quadratic taps */
  ret ^= p[109]&p[119];
  ret -= p[128]|p[9];
  ret -= p[7]|p[223];

  return(ret);
}


local int munge126(register int *p)
{
  register int ret;

  ret = -1929041652;

/* linear taps */
  ret ^= p[210];
  ret ^= p[97];
  ret ^= p[59];
  ret ^= p[183];
  ret ^= p[223];
  ret ^= p[90];

/* quadratic taps */
  ret ^= p[114]&p[129];
  ret -= p[219]&p[158];
  ret ^= p[94]&p[43];

  return(ret);
}


local int munge127(register int *p)
{
  register int ret;

  ret = 661850821;

/* linear taps */
  ret ^= p[248];
  ret ^= p[258];
  ret += p[87];
  ret += p[102];
  ret ^= p[268];
  ret += p[252];

/* quadratic taps */
  ret ^= p[231]|p[232];
  ret += p[113]&p[223];
  ret -= p[230]&p[172];

  return(ret);
}


local int munge128(register int *p)
{
  register int ret;

  ret = -1431278089;

/* linear taps */
  ret += p[181];
  ret ^= p[215];
  ret ^= p[92];
  ret -= p[21];
  ret += p[172];
  ret += p[271];

/* quadratic taps */
  ret -= p[266]|p[5];
  ret ^= p[105]|p[6];
  ret += p[102]|p[121];

  return(ret);
}


local int munge129(register int *p)
{
  register int ret;

  ret = -1000215612;

/* linear taps */
  ret += p[178];
  ret += p[189];
  ret += p[227];
  ret ^= p[289];
  ret ^= p[27];
  ret ^= p[71];

/* quadratic taps */
  ret -= p[229]|p[128];
  ret -= p[250]&p[109];
  ret ^= p[254]&p[199];

  return(ret);
}


local int munge130(register int *p)
{
  register int ret;

  ret = 823361991;

/* linear taps */
  ret -= p[214];
  ret ^= p[227];
  ret ^= p[123];
  ret += p[47];
  ret ^= p[19];
  ret ^= p[116];

/* quadratic taps */
  ret ^= p[153]&p[243];
  ret ^= p[247]&p[97];
  ret += p[163]&p[38];

  return(ret);
}


local int munge131(register int *p)
{
  register int ret;

  ret = 40061972;

/* linear taps */
  ret ^= p[162];
  ret ^= p[250];
  ret ^= p[150];
  ret += p[174];
  ret -= p[154];
  ret ^= p[252];

/* quadratic taps */
  ret ^= p[127]&p[29];
  ret ^= p[30]&p[292];
  ret ^= p[291]&p[50];

  return(ret);
}


local int munge132(register int *p)
{
  register int ret;

  ret = 1382109801;

/* linear taps */
  ret -= p[198];
  ret -= p[213];
  ret ^= p[258];
  ret ^= p[138];
  ret += p[131];
  ret ^= p[93];

/* quadratic taps */
  ret ^= p[200]&p[235];
  ret -= p[71]&p[248];
  ret ^= p[238]|p[259];

  return(ret);
}


local int munge133(register int *p)
{
  register int ret;

  ret = 1892413029;

/* linear taps */
  ret ^= p[249];
  ret -= p[200];
  ret ^= p[185];
  ret ^= p[77];
  ret ^= p[138];
  ret ^= p[102];

/* quadratic taps */
  ret += p[124]&p[220];
  ret -= p[131]|p[245];
  ret -= p[256]&p[28];

  return(ret);
}


local int munge134(register int *p)
{
  register int ret;

  ret = -1861172746;

/* linear taps */
  ret += p[123];
  ret ^= p[142];
  ret += p[208];
  ret ^= p[54];
  ret ^= p[161];
  ret += p[15];

/* quadratic taps */
  ret ^= p[242]|p[66];
  ret -= p[227]&p[229];
  ret ^= p[127]|p[150];

  return(ret);
}


local int munge135(register int *p)
{
  register int ret;

  ret = 82529575;

/* linear taps */
  ret ^= p[211];
  ret -= p[220];
  ret -= p[82];
  ret ^= p[146];
  ret ^= p[207];
  ret += p[74];

/* quadratic taps */
  ret ^= p[12]|p[158];
  ret += p[52]|p[184];
  ret ^= p[176]&p[84];

  return(ret);
}


local int munge136(register int *p)
{
  register int ret;

  ret = 544100015;

/* linear taps */
  ret ^= p[198];
  ret ^= p[38];
  ret ^= p[267];
  ret -= p[145];
  ret ^= p[263];
  ret ^= p[175];

/* quadratic taps */
  ret += p[158]|p[164];
  ret -= p[128]&p[255];
  ret ^= p[81]|p[100];

  return(ret);
}


local int munge137(register int *p)
{
  register int ret;

  ret = 440123439;

/* linear taps */
  ret += p[176];
  ret -= p[26];
  ret -= p[18];
  ret ^= p[183];
  ret ^= p[155];
  ret ^= p[221];

/* quadratic taps */
  ret -= p[8]|p[123];
  ret ^= p[184]&p[136];
  ret ^= p[277]&p[134];

  return(ret);
}


local int munge138(register int *p)
{
  register int ret;

  ret = -712248455;

/* linear taps */
  ret ^= p[136];
  ret ^= p[164];
  ret += p[212];
  ret ^= p[203];
  ret ^= p[260];
  ret -= p[107];

/* quadratic taps */
  ret += p[44]|p[191];
  ret ^= p[84]|p[139];
  ret ^= p[234]|p[151];

  return(ret);
}


local int munge139(register int *p)
{
  register int ret;

  ret = 2101808142;

/* linear taps */
  ret -= p[279];
  ret += p[99];
  ret ^= p[60];
  ret ^= p[89];
  ret += p[44];
  ret -= p[165];

/* quadratic taps */
  ret += p[70]&p[154];
  ret ^= p[136]&p[246];
  ret -= p[134]|p[189];

  return(ret);
}


local int munge140(register int *p)
{
  register int ret;

  ret = -2056258944;

/* linear taps */
  ret += p[79];
  ret ^= p[140];
  ret += p[55];
  ret -= p[213];
  ret ^= p[186];
  ret -= p[138];

/* quadratic taps */
  ret ^= p[77]&p[161];
  ret ^= p[42]|p[65];
  ret ^= p[128]|p[81];

  return(ret);
}


local int munge141(register int *p)
{
  register int ret;

  ret = 1946503952;

/* linear taps */
  ret += p[45];
  ret ^= p[17];
  ret ^= p[227];
  ret -= p[9];
  ret += p[153];
  ret ^= p[98];

/* quadratic taps */
  ret += p[256]&p[205];
  ret -= p[100]&p[224];
  ret += p[247]|p[275];

  return(ret);
}


local int munge142(register int *p)
{
  register int ret;

  ret = 278389792;

/* linear taps */
  ret ^= p[239];
  ret -= p[190];
  ret -= p[118];
  ret -= p[10];
  ret += p[76];
  ret -= p[35];

/* quadratic taps */
  ret -= p[77]|p[171];
  ret -= p[91]|p[268];
  ret -= p[292]&p[284];

  return(ret);
}


local int munge143(register int *p)
{
  register int ret;

  ret = 319390707;

/* linear taps */
  ret -= p[214];
  ret ^= p[6];
  ret ^= p[20];
  ret ^= p[115];
  ret -= p[38];
  ret -= p[268];

/* quadratic taps */
  ret += p[54]|p[258];
  ret ^= p[91]&p[90];
  ret += p[230]&p[202];

  return(ret);
}


local int munge144(register int *p)
{
  register int ret;

  ret = 53499191;

/* linear taps */
  ret += p[187];
  ret ^= p[128];
  ret ^= p[25];
  ret -= p[172];
  ret ^= p[283];
  ret -= p[267];

/* quadratic taps */
  ret ^= p[37]&p[194];
  ret ^= p[215]&p[121];
  ret ^= p[197]|p[8];

  return(ret);
}


local int munge145(register int *p)
{
  register int ret;

  ret = -645371964;

/* linear taps */
  ret ^= p[187];
  ret -= p[133];
  ret ^= p[8];
  ret ^= p[274];
  ret ^= p[16];
  ret -= p[255];

/* quadratic taps */
  ret -= p[244]&p[19];
  ret -= p[65]|p[160];
  ret -= p[68]|p[105];

  return(ret);
}


local int munge146(register int *p)
{
  register int ret;

  ret = -1856165192;

/* linear taps */
  ret += p[266];
  ret -= p[254];
  ret -= p[290];
  ret += p[116];
  ret -= p[18];
  ret ^= p[247];

/* quadratic taps */
  ret ^= p[200]&p[25];
  ret -= p[179]&p[103];
  ret ^= p[32]|p[217];

  return(ret);
}


local int munge147(register int *p)
{
  register int ret;

  ret = 203173391;

/* linear taps */
  ret -= p[192];
  ret -= p[52];
  ret ^= p[25];
  ret ^= p[115];
  ret += p[156];
  ret ^= p[209];

/* quadratic taps */
  ret ^= p[190]&p[271];
  ret ^= p[148]&p[30];
  ret ^= p[76]&p[167];

  return(ret);
}


local int munge148(register int *p)
{
  register int ret;

  ret = -465600592;

/* linear taps */
  ret ^= p[273];
  ret -= p[56];
  ret += p[281];
  ret ^= p[248];
  ret += p[291];
  ret += p[125];

/* quadratic taps */
  ret += p[107]|p[13];
  ret ^= p[236]|p[224];
  ret -= p[149]&p[259];

  return(ret);
}


local int munge149(register int *p)
{
  register int ret;

  ret = 689582680;

/* linear taps */
  ret ^= p[6];
  ret ^= p[169];
  ret ^= p[157];
  ret ^= p[155];
  ret ^= p[12];
  ret += p[23];

/* quadratic taps */
  ret -= p[86]&p[225];
  ret ^= p[26]|p[106];
  ret += p[141]|p[218];

  return(ret);
}


local int munge150(register int *p)
{
  register int ret;

  ret = -2091515706;

/* linear taps */
  ret += p[178];
  ret ^= p[218];
  ret ^= p[135];
  ret += p[107];
  ret ^= p[223];
  ret -= p[94];

/* quadratic taps */
  ret ^= p[28]|p[238];
  ret -= p[26]&p[261];
  ret ^= p[52]|p[181];

  return(ret);
}


local int munge151(register int *p)
{
  register int ret;

  ret = -1377995745;

/* linear taps */
  ret ^= p[279];
  ret ^= p[287];
  ret ^= p[216];
  ret ^= p[252];
  ret -= p[124];
  ret -= p[169];

/* quadratic taps */
  ret += p[106]|p[218];
  ret ^= p[213]|p[272];
  ret -= p[248]|p[82];

  return(ret);
}


local int munge152(register int *p)
{
  register int ret;

  ret = 1281522558;

/* linear taps */
  ret ^= p[36];
  ret += p[5];
  ret ^= p[69];
  ret += p[230];
  ret -= p[223];
  ret += p[131];

/* quadratic taps */
  ret += p[161]|p[113];
  ret += p[220]&p[191];
  ret -= p[68]|p[102];

  return(ret);
}


local int munge153(register int *p)
{
  register int ret;

  ret = -2102592720;

/* linear taps */
  ret ^= p[33];
  ret -= p[83];
  ret -= p[258];
  ret -= p[140];
  ret -= p[79];
  ret -= p[75];

/* quadratic taps */
  ret -= p[160]|p[16];
  ret += p[195]|p[151];
  ret -= p[88]&p[62];

  return(ret);
}


local int munge154(register int *p)
{
  register int ret;

  ret = -1073468178;

/* linear taps */
  ret -= p[168];
  ret -= p[32];
  ret ^= p[274];
  ret -= p[78];
  ret += p[18];
  ret += p[213];

/* quadratic taps */
  ret ^= p[99]|p[268];
  ret ^= p[239]|p[109];
  ret ^= p[42]|p[205];

  return(ret);
}


local int munge155(register int *p)
{
  register int ret;

  ret = 1324528699;

/* linear taps */
  ret += p[151];
  ret -= p[198];
  ret ^= p[224];
  ret -= p[6];
  ret -= p[179];
  ret ^= p[252];

/* quadratic taps */
  ret ^= p[187]|p[163];
  ret ^= p[177]|p[46];
  ret -= p[222]|p[126];

  return(ret);
}


local int munge156(register int *p)
{
  register int ret;

  ret = 1484517421;

/* linear taps */
  ret ^= p[56];
  ret += p[136];
  ret += p[230];
  ret ^= p[90];
  ret += p[91];
  ret += p[110];

/* quadratic taps */
  ret -= p[176]|p[206];
  ret -= p[174]&p[138];
  ret ^= p[46]|p[111];

  return(ret);
}


local int munge157(register int *p)
{
  register int ret;

  ret = -1563484937;

/* linear taps */
  ret += p[155];
  ret += p[105];
  ret += p[158];
  ret -= p[236];
  ret += p[216];
  ret -= p[202];

/* quadratic taps */
  ret += p[167]|p[76];
  ret += p[237]&p[287];
  ret -= p[231]&p[96];

  return(ret);
}


local int munge158(register int *p)
{
  register int ret;

  ret = -2064024386;

/* linear taps */
  ret ^= p[91];
  ret -= p[10];
  ret -= p[179];
  ret ^= p[201];
  ret ^= p[156];
  ret -= p[116];

/* quadratic taps */
  ret += p[164]|p[69];
  ret += p[178]&p[9];
  ret ^= p[40]&p[273];

  return(ret);
}


local int munge159(register int *p)
{
  register int ret;

  ret = -318197924;

/* linear taps */
  ret ^= p[38];
  ret -= p[157];
  ret -= p[120];
  ret += p[127];
  ret ^= p[32];
  ret -= p[107];

/* quadratic taps */
  ret ^= p[87]|p[134];
  ret ^= p[55]&p[193];
  ret ^= p[47]&p[91];

  return(ret);
}


local int munge160(register int *p)
{
  register int ret;

  ret = -2017201293;

/* linear taps */
  ret -= p[115];
  ret ^= p[121];
  ret ^= p[11];
  ret -= p[62];
  ret ^= p[292];
  ret ^= p[3];

/* quadratic taps */
  ret += p[85]|p[196];
  ret ^= p[129]|p[147];
  ret ^= p[137]|p[38];

  return(ret);
}


local int munge161(register int *p)
{
  register int ret;

  ret = -317945827;

/* linear taps */
  ret ^= p[188];
  ret ^= p[147];
  ret ^= p[165];
  ret -= p[107];
  ret -= p[90];
  ret ^= p[215];

/* quadratic taps */
  ret += p[233]|p[220];
  ret ^= p[59]|p[291];
  ret += p[200]&p[43];

  return(ret);
}


local int munge162(register int *p)
{
  register int ret;

  ret = 1020190598;

/* linear taps */
  ret ^= p[200];
  ret ^= p[10];
  ret += p[24];
  ret ^= p[75];
  ret ^= p[184];
  ret ^= p[264];

/* quadratic taps */
  ret += p[249]|p[58];
  ret ^= p[49]&p[5];
  ret -= p[146]&p[195];

  return(ret);
}


local int munge163(register int *p)
{
  register int ret;

  ret = 750656653;

/* linear taps */
  ret += p[276];
  ret -= p[105];
  ret ^= p[128];
  ret ^= p[133];
  ret -= p[146];
  ret ^= p[42];

/* quadratic taps */
  ret ^= p[169]|p[101];
  ret ^= p[94]&p[187];
  ret += p[76]&p[35];

  return(ret);
}


local int munge164(register int *p)
{
  register int ret;

  ret = -1552149646;

/* linear taps */
  ret ^= p[257];
  ret ^= p[231];
  ret ^= p[180];
  ret += p[29];
  ret ^= p[56];
  ret -= p[167];

/* quadratic taps */
  ret ^= p[280]|p[172];
  ret -= p[237]&p[98];
  ret ^= p[1]&p[126];

  return(ret);
}


local int munge165(register int *p)
{
  register int ret;

  ret = -1750692392;

/* linear taps */
  ret ^= p[46];
  ret -= p[257];
  ret ^= p[42];
  ret -= p[286];
  ret -= p[151];
  ret += p[32];

/* quadratic taps */
  ret += p[114]&p[95];
  ret -= p[55]&p[169];
  ret += p[219]&p[115];

  return(ret);
}


local int munge166(register int *p)
{
  register int ret;

  ret = -320485163;

/* linear taps */
  ret -= p[235];
  ret ^= p[187];
  ret -= p[254];
  ret -= p[54];
  ret ^= p[220];
  ret ^= p[118];

/* quadratic taps */
  ret ^= p[280]|p[216];
  ret ^= p[15]|p[225];
  ret -= p[67]|p[109];

  return(ret);
}


local int munge167(register int *p)
{
  register int ret;

  ret = 577533459;

/* linear taps */
  ret -= p[72];
  ret -= p[219];
  ret -= p[221];
  ret -= p[106];
  ret += p[45];
  ret += p[86];

/* quadratic taps */
  ret ^= p[154]&p[242];
  ret -= p[100]|p[11];
  ret ^= p[173]&p[183];

  return(ret);
}


local int munge168(register int *p)
{
  register int ret;

  ret = 1944046516;

/* linear taps */
  ret -= p[260];
  ret ^= p[121];
  ret ^= p[10];
  ret ^= p[142];
  ret += p[132];
  ret -= p[193];

/* quadratic taps */
  ret ^= p[150]&p[2];
  ret += p[185]&p[288];
  ret ^= p[265]|p[40];

  return(ret);
}


local int munge169(register int *p)
{
  register int ret;

  ret = 980597533;

/* linear taps */
  ret ^= p[288];
  ret -= p[2];
  ret -= p[106];
  ret ^= p[118];
  ret -= p[81];
  ret ^= p[117];

/* quadratic taps */
  ret += p[276]|p[233];
  ret -= p[280]|p[216];
  ret -= p[261]&p[98];

  return(ret);
}


local int munge170(register int *p)
{
  register int ret;

  ret = -1335731358;

/* linear taps */
  ret -= p[288];
  ret ^= p[51];
  ret += p[263];
  ret -= p[171];
  ret -= p[209];
  ret -= p[80];

/* quadratic taps */
  ret ^= p[6]|p[235];
  ret ^= p[174]&p[257];
  ret -= p[84]&p[111];

  return(ret);
}


local int munge171(register int *p)
{
  register int ret;

  ret = 2004168726;

/* linear taps */
  ret ^= p[26];
  ret ^= p[57];
  ret ^= p[88];
  ret ^= p[56];
  ret -= p[168];
  ret -= p[175];

/* quadratic taps */
  ret -= p[36]|p[94];
  ret -= p[284]&p[273];
  ret ^= p[4]&p[119];

  return(ret);
}


local int munge172(register int *p)
{
  register int ret;

  ret = 1598718097;

/* linear taps */
  ret ^= p[53];
  ret += p[216];
  ret ^= p[228];
  ret ^= p[147];
  ret ^= p[186];
  ret += p[143];

/* quadratic taps */
  ret -= p[246]&p[39];
  ret ^= p[14]|p[59];
  ret += p[238]&p[145];

  return(ret);
}


local int munge173(register int *p)
{
  register int ret;

  ret = -1842299965;

/* linear taps */
  ret ^= p[283];
  ret ^= p[132];
  ret ^= p[162];
  ret += p[180];
  ret ^= p[138];
  ret ^= p[79];

/* quadratic taps */
  ret ^= p[271]&p[261];
  ret -= p[187]&p[150];
  ret -= p[286]&p[133];

  return(ret);
}


local int munge174(register int *p)
{
  register int ret;

  ret = 1076727065;

/* linear taps */
  ret ^= p[119];
  ret += p[46];
  ret ^= p[245];
  ret -= p[189];
  ret ^= p[12];
  ret ^= p[36];

/* quadratic taps */
  ret ^= p[18]&p[85];
  ret ^= p[205]|p[122];
  ret -= p[222]|p[130];

  return(ret);
}


local int munge175(register int *p)
{
  register int ret;

  ret = 1031183844;

/* linear taps */
  ret ^= p[259];
  ret += p[272];
  ret ^= p[208];
  ret ^= p[237];
  ret -= p[137];
  ret += p[132];

/* quadratic taps */
  ret ^= p[87]|p[67];
  ret -= p[6]&p[157];
  ret -= p[233]&p[252];

  return(ret);
}


local int munge176(register int *p)
{
  register int ret;

  ret = -2025595622;

/* linear taps */
  ret += p[134];
  ret -= p[128];
  ret ^= p[118];
  ret += p[1];
  ret ^= p[148];
  ret ^= p[225];

/* quadratic taps */
  ret -= p[286]|p[264];
  ret += p[60]|p[235];
  ret += p[115]&p[160];

  return(ret);
}


local int munge177(register int *p)
{
  register int ret;

  ret = 710330957;

/* linear taps */
  ret += p[59];
  ret -= p[256];
  ret -= p[182];
  ret ^= p[159];
  ret -= p[36];
  ret ^= p[14];

/* quadratic taps */
  ret -= p[132]&p[104];
  ret ^= p[106]&p[291];
  ret ^= p[8]&p[66];

  return(ret);
}


local int munge178(register int *p)
{
  register int ret;

  ret = 850915474;

/* linear taps */
  ret ^= p[125];
  ret -= p[5];
  ret += p[100];
  ret -= p[218];
  ret -= p[285];
  ret ^= p[16];

/* quadratic taps */
  ret ^= p[47]&p[251];
  ret += p[280]|p[201];
  ret ^= p[148]&p[119];

  return(ret);
}


local int munge179(register int *p)
{
  register int ret;

  ret = 1154838062;

/* linear taps */
  ret -= p[181];
  ret += p[119];
  ret ^= p[198];
  ret ^= p[4];
  ret -= p[49];
  ret += p[85];

/* quadratic taps */
  ret += p[96]|p[184];
  ret -= p[194]&p[132];
  ret ^= p[9]&p[214];

  return(ret);
}


local int munge180(register int *p)
{
  register int ret;

  ret = 1998368962;

/* linear taps */
  ret ^= p[234];
  ret ^= p[290];
  ret -= p[289];
  ret += p[90];
  ret ^= p[48];
  ret += p[14];

/* quadratic taps */
  ret ^= p[198]|p[118];
  ret ^= p[129]|p[246];
  ret += p[116]|p[19];

  return(ret);
}


local int munge181(register int *p)
{
  register int ret;

  ret = -1813849930;

/* linear taps */
  ret -= p[50];
  ret += p[133];
  ret += p[220];
  ret -= p[100];
  ret += p[159];
  ret ^= p[81];

/* quadratic taps */
  ret ^= p[209]|p[170];
  ret ^= p[239]|p[7];
  ret -= p[27]&p[132];

  return(ret);
}


local int munge182(register int *p)
{
  register int ret;

  ret = -872462658;

/* linear taps */
  ret -= p[174];
  ret -= p[136];
  ret -= p[82];
  ret ^= p[73];
  ret ^= p[157];
  ret ^= p[218];

/* quadratic taps */
  ret ^= p[134]|p[171];
  ret ^= p[80]&p[68];
  ret ^= p[231]&p[188];

  return(ret);
}


local int munge183(register int *p)
{
  register int ret;

  ret = -220095317;

/* linear taps */
  ret ^= p[254];
  ret ^= p[260];
  ret += p[208];
  ret ^= p[80];
  ret -= p[35];
  ret -= p[134];

/* quadratic taps */
  ret -= p[41]|p[129];
  ret ^= p[222]&p[221];
  ret ^= p[226]|p[227];

  return(ret);
}


local int munge184(register int *p)
{
  register int ret;

  ret = -2136922730;

/* linear taps */
  ret ^= p[96];
  ret ^= p[185];
  ret ^= p[184];
  ret ^= p[230];
  ret -= p[267];
  ret ^= p[276];

/* quadratic taps */
  ret += p[236]|p[47];
  ret ^= p[112]|p[164];
  ret -= p[187]&p[45];

  return(ret);
}


local int munge185(register int *p)
{
  register int ret;

  ret = -1253132071;

/* linear taps */
  ret += p[229];
  ret += p[96];
  ret += p[273];
  ret ^= p[233];
  ret -= p[81];
  ret ^= p[270];

/* quadratic taps */
  ret ^= p[231]|p[207];
  ret -= p[87]|p[68];
  ret += p[149]&p[104];

  return(ret);
}


local int munge186(register int *p)
{
  register int ret;

  ret = 227998933;

/* linear taps */
  ret -= p[248];
  ret ^= p[21];
  ret ^= p[229];
  ret += p[47];
  ret ^= p[59];
  ret -= p[246];

/* quadratic taps */
  ret ^= p[181]&p[90];
  ret ^= p[159]|p[143];
  ret += p[17]&p[70];

  return(ret);
}


local int munge187(register int *p)
{
  register int ret;

  ret = 1625133396;

/* linear taps */
  ret += p[98];
  ret ^= p[60];
  ret ^= p[186];
  ret -= p[105];
  ret ^= p[193];
  ret += p[50];

/* quadratic taps */
  ret -= p[284]&p[160];
  ret ^= p[89]|p[22];
  ret -= p[115]&p[245];

  return(ret);
}


local int munge188(register int *p)
{
  register int ret;

  ret = 292374563;

/* linear taps */
  ret -= p[44];
  ret -= p[218];
  ret ^= p[217];
  ret += p[281];
  ret -= p[254];
  ret += p[86];

/* quadratic taps */
  ret += p[37]&p[110];
  ret ^= p[208]|p[112];
  ret += p[145]|p[73];

  return(ret);
}


local int munge189(register int *p)
{
  register int ret;

  ret = 1056804068;

/* linear taps */
  ret ^= p[216];
  ret ^= p[35];
  ret ^= p[125];
  ret ^= p[188];
  ret += p[277];
  ret ^= p[86];

/* quadratic taps */
  ret -= p[269]&p[138];
  ret += p[111]|p[274];
  ret ^= p[168]&p[237];

  return(ret);
}


local int munge190(register int *p)
{
  register int ret;

  ret = 533935115;

/* linear taps */
  ret ^= p[50];
  ret += p[287];
  ret ^= p[26];
  ret ^= p[268];
  ret += p[67];
  ret -= p[262];

/* quadratic taps */
  ret ^= p[69]|p[210];
  ret += p[135]|p[8];
  ret ^= p[14]|p[266];

  return(ret);
}


local int munge191(register int *p)
{
  register int ret;

  ret = 1234183967;

/* linear taps */
  ret ^= p[107];
  ret -= p[32];
  ret -= p[205];
  ret ^= p[161];
  ret -= p[215];
  ret -= p[44];

/* quadratic taps */
  ret -= p[223]|p[80];
  ret += p[70]|p[236];
  ret ^= p[65]|p[90];

  return(ret);
}


local int munge192(register int *p)
{
  register int ret;

  ret = -11248321;

/* linear taps */
  ret ^= p[5];
  ret ^= p[276];
  ret ^= p[164];
  ret ^= p[54];
  ret += p[176];
  ret += p[42];

/* quadratic taps */
  ret += p[255]|p[202];
  ret ^= p[58]|p[248];
  ret ^= p[228]&p[286];

  return(ret);
}


local int munge193(register int *p)
{
  register int ret;

  ret = 1196963436;

/* linear taps */
  ret -= p[237];
  ret ^= p[44];
  ret -= p[93];
  ret ^= p[68];
  ret ^= p[134];
  ret += p[127];

/* quadratic taps */
  ret += p[162]|p[196];
  ret ^= p[185]&p[242];
  ret ^= p[14]|p[151];

  return(ret);
}


local int munge194(register int *p)
{
  register int ret;

  ret = 247108509;

/* linear taps */
  ret ^= p[134];
  ret ^= p[97];
  ret ^= p[45];
  ret ^= p[274];
  ret ^= p[142];
  ret -= p[143];

/* quadratic taps */
  ret ^= p[128]|p[225];
  ret ^= p[74]|p[204];
  ret += p[207]|p[208];

  return(ret);
}


local int munge195(register int *p)
{
  register int ret;

  ret = 1010688656;

/* linear taps */
  ret -= p[187];
  ret -= p[111];
  ret ^= p[194];
  ret -= p[118];
  ret ^= p[245];
  ret -= p[120];

/* quadratic taps */
  ret ^= p[47]|p[198];
  ret ^= p[115]&p[233];
  ret += p[130]&p[19];

  return(ret);
}


local int munge196(register int *p)
{
  register int ret;

  ret = 620466223;

/* linear taps */
  ret ^= p[33];
  ret += p[203];
  ret ^= p[100];
  ret ^= p[197];
  ret ^= p[96];
  ret += p[265];

/* quadratic taps */
  ret += p[270]|p[57];
  ret += p[42]&p[29];
  ret += p[156]&p[39];

  return(ret);
}


local int munge197(register int *p)
{
  register int ret;

  ret = -1189978276;

/* linear taps */
  ret += p[91];
  ret += p[12];
  ret -= p[45];
  ret += p[126];
  ret -= p[56];
  ret ^= p[96];

/* quadratic taps */
  ret += p[230]|p[173];
  ret ^= p[207]&p[105];
  ret ^= p[19]|p[42];

  return(ret);
}


local int munge198(register int *p)
{
  register int ret;

  ret = -1894674148;

/* linear taps */
  ret ^= p[138];
  ret ^= p[237];
  ret -= p[184];
  ret ^= p[59];
  ret += p[173];
  ret ^= p[249];

/* quadratic taps */
  ret += p[215]|p[191];
  ret += p[277]&p[98];
  ret += p[114]&p[14];

  return(ret);
}


local int munge199(register int *p)
{
  register int ret;

  ret = 674746451;

/* linear taps */
  ret += p[94];
  ret += p[114];
  ret += p[128];
  ret ^= p[202];
  ret -= p[197];
  ret -= p[27];

/* quadratic taps */
  ret ^= p[176]|p[28];
  ret += p[222]|p[213];
  ret ^= p[206]|p[245];

  return(ret);
}


local int munge200(register int *p)
{
  register int ret;

  ret = -1621495137;

/* linear taps */
  ret -= p[19];
  ret ^= p[10];
  ret -= p[225];
  ret ^= p[292];
  ret ^= p[13];
  ret ^= p[215];

/* quadratic taps */
  ret ^= p[135]&p[163];
  ret += p[95]|p[174];
  ret ^= p[134]&p[25];

  return(ret);
}


local int munge201(register int *p)
{
  register int ret;

  ret = -1728786039;

/* linear taps */
  ret += p[2];
  ret ^= p[145];
  ret -= p[60];
  ret += p[77];
  ret ^= p[28];
  ret ^= p[151];

/* quadratic taps */
  ret ^= p[283]&p[107];
  ret -= p[289]&p[99];
  ret += p[268]&p[270];

  return(ret);
}


local int munge202(register int *p)
{
  register int ret;

  ret = 1618429117;

/* linear taps */
  ret ^= p[291];
  ret -= p[82];
  ret ^= p[276];
  ret ^= p[118];
  ret ^= p[206];
  ret ^= p[41];

/* quadratic taps */
  ret -= p[153]|p[84];
  ret ^= p[258]|p[199];
  ret += p[242]|p[183];

  return(ret);
}


local int munge203(register int *p)
{
  register int ret;

  ret = 186157212;

/* linear taps */
  ret ^= p[247];
  ret ^= p[169];
  ret -= p[21];
  ret ^= p[107];
  ret ^= p[242];
  ret -= p[157];

/* quadratic taps */
  ret ^= p[99]|p[85];
  ret += p[31]&p[221];
  ret -= p[266]&p[204];

  return(ret);
}


local int munge204(register int *p)
{
  register int ret;

  ret = 370166861;

/* linear taps */
  ret += p[273];
  ret += p[68];
  ret ^= p[278];
  ret += p[153];
  ret ^= p[152];
  ret ^= p[105];

/* quadratic taps */
  ret += p[168]|p[33];
  ret ^= p[246]&p[195];
  ret += p[163]|p[269];

  return(ret);
}


local int munge205(register int *p)
{
  register int ret;

  ret = 504195447;

/* linear taps */
  ret -= p[18];
  ret ^= p[175];
  ret += p[66];
  ret ^= p[170];
  ret -= p[266];
  ret ^= p[177];

/* quadratic taps */
  ret += p[153]&p[10];
  ret -= p[27]&p[102];
  ret ^= p[25]&p[97];

  return(ret);
}


local int munge206(register int *p)
{
  register int ret;

  ret = 46268623;

/* linear taps */
  ret -= p[66];
  ret ^= p[151];
  ret ^= p[195];
  ret -= p[237];
  ret ^= p[13];
  ret ^= p[72];

/* quadratic taps */
  ret ^= p[104]&p[282];
  ret += p[286]|p[126];
  ret ^= p[19]|p[83];

  return(ret);
}


local int munge207(register int *p)
{
  register int ret;

  ret = 1986068868;

/* linear taps */
  ret ^= p[275];
  ret += p[207];
  ret += p[115];
  ret += p[94];
  ret ^= p[174];
  ret ^= p[291];

/* quadratic taps */
  ret ^= p[53]&p[226];
  ret ^= p[113]&p[241];
  ret += p[12]|p[36];

  return(ret);
}


local int munge208(register int *p)
{
  register int ret;

  ret = 822466085;

/* linear taps */
  ret ^= p[258];
  ret -= p[128];
  ret ^= p[253];
  ret -= p[58];
  ret -= p[117];
  ret -= p[164];

/* quadratic taps */
  ret ^= p[198]&p[30];
  ret ^= p[90]|p[155];
  ret ^= p[39]&p[71];

  return(ret);
}


local int munge209(register int *p)
{
  register int ret;

  ret = -1123128440;

/* linear taps */
  ret += p[18];
  ret += p[95];
  ret += p[241];
  ret ^= p[266];
  ret -= p[84];
  ret += p[202];

/* quadratic taps */
  ret -= p[99]&p[290];
  ret -= p[244]&p[55];
  ret ^= p[196]&p[80];

  return(ret);
}


local int munge210(register int *p)
{
  register int ret;

  ret = -1144608896;

/* linear taps */
  ret ^= p[196];
  ret ^= p[36];
  ret -= p[34];
  ret ^= p[279];
  ret ^= p[121];
  ret ^= p[204];

/* quadratic taps */
  ret -= p[115]&p[288];
  ret ^= p[54]|p[55];
  ret -= p[96]|p[264];

  return(ret);
}


local int munge211(register int *p)
{
  register int ret;

  ret = 816951187;

/* linear taps */
  ret ^= p[177];
  ret += p[289];
  ret += p[83];
  ret -= p[61];
  ret += p[237];
  ret += p[98];

/* quadratic taps */
  ret += p[208]|p[205];
  ret -= p[34]&p[158];
  ret -= p[223]|p[151];

  return(ret);
}


local int munge212(register int *p)
{
  register int ret;

  ret = -1624534242;

/* linear taps */
  ret -= p[216];
  ret += p[239];
  ret -= p[280];
  ret -= p[150];
  ret ^= p[21];
  ret += p[292];

/* quadratic taps */
  ret ^= p[15]&p[70];
  ret -= p[234]&p[66];
  ret ^= p[118]|p[174];

  return(ret);
}


local int munge213(register int *p)
{
  register int ret;

  ret = -1992724071;

/* linear taps */
  ret -= p[269];
  ret -= p[155];
  ret ^= p[223];
  ret += p[261];
  ret ^= p[33];
  ret ^= p[237];

/* quadratic taps */
  ret -= p[108]|p[201];
  ret += p[15]|p[125];
  ret ^= p[58]|p[96];

  return(ret);
}


local int munge214(register int *p)
{
  register int ret;

  ret = -788506396;

/* linear taps */
  ret += p[204];
  ret ^= p[271];
  ret += p[38];
  ret -= p[15];
  ret += p[159];
  ret -= p[135];

/* quadratic taps */
  ret ^= p[100]|p[121];
  ret ^= p[41]|p[18];
  ret ^= p[275]|p[247];

  return(ret);
}


local int munge215(register int *p)
{
  register int ret;

  ret = 931193928;

/* linear taps */
  ret ^= p[145];
  ret -= p[132];
  ret ^= p[13];
  ret += p[94];
  ret += p[118];
  ret ^= p[74];

/* quadratic taps */
  ret ^= p[17]|p[161];
  ret ^= p[11]|p[122];
  ret ^= p[252]|p[193];

  return(ret);
}


local int munge216(register int *p)
{
  register int ret;

  ret = -2030159481;

/* linear taps */
  ret ^= p[52];
  ret ^= p[174];
  ret ^= p[251];
  ret -= p[241];
  ret ^= p[31];
  ret ^= p[291];

/* quadratic taps */
  ret ^= p[247]|p[143];
  ret ^= p[62]|p[22];
  ret += p[191]&p[172];

  return(ret);
}


local int munge217(register int *p)
{
  register int ret;

  ret = 870254141;

/* linear taps */
  ret ^= p[131];
  ret += p[13];
  ret ^= p[51];
  ret ^= p[204];
  ret ^= p[289];
  ret -= p[90];

/* quadratic taps */
  ret += p[68]&p[220];
  ret += p[55]|p[88];
  ret += p[58]|p[164];

  return(ret);
}


local int munge218(register int *p)
{
  register int ret;

  ret = 2135364626;

/* linear taps */
  ret -= p[173];
  ret ^= p[78];
  ret -= p[203];
  ret ^= p[270];
  ret ^= p[131];
  ret ^= p[22];

/* quadratic taps */
  ret += p[163]|p[129];
  ret += p[97]|p[257];
  ret ^= p[26]|p[155];

  return(ret);
}


local int munge219(register int *p)
{
  register int ret;

  ret = 73983957;

/* linear taps */
  ret += p[236];
  ret ^= p[54];
  ret ^= p[94];
  ret ^= p[153];
  ret -= p[55];
  ret ^= p[284];

/* quadratic taps */
  ret += p[13]|p[101];
  ret ^= p[133]|p[262];
  ret -= p[56]&p[71];

  return(ret);
}


local int munge220(register int *p)
{
  register int ret;

  ret = -1992265146;

/* linear taps */
  ret ^= p[121];
  ret -= p[38];
  ret ^= p[75];
  ret += p[84];
  ret += p[263];
  ret += p[36];

/* quadratic taps */
  ret += p[55]&p[223];
  ret += p[144]&p[128];
  ret -= p[202]&p[63];

  return(ret);
}


local int munge221(register int *p)
{
  register int ret;

  ret = -1600572614;

/* linear taps */
  ret ^= p[59];
  ret ^= p[99];
  ret ^= p[3];
  ret -= p[217];
  ret -= p[4];
  ret ^= p[119];

/* quadratic taps */
  ret ^= p[198]|p[42];
  ret -= p[111]|p[125];
  ret ^= p[19]&p[291];

  return(ret);
}


local int munge222(register int *p)
{
  register int ret;

  ret = 535648471;

/* linear taps */
  ret -= p[243];
  ret -= p[80];
  ret += p[136];
  ret ^= p[278];
  ret -= p[142];
  ret ^= p[63];

/* quadratic taps */
  ret -= p[3]&p[208];
  ret ^= p[130]|p[114];
  ret -= p[61]&p[18];

  return(ret);
}


local int munge223(register int *p)
{
  register int ret;

  ret = -708564893;

/* linear taps */
  ret += p[157];
  ret += p[51];
  ret ^= p[187];
  ret ^= p[106];
  ret ^= p[109];
  ret += p[131];

/* quadratic taps */
  ret += p[147]&p[153];
  ret += p[222]|p[226];
  ret += p[208]|p[258];

  return(ret);
}


local int munge224(register int *p)
{
  register int ret;

  ret = 2034261741;

/* linear taps */
  ret -= p[30];
  ret ^= p[226];
  ret -= p[21];
  ret ^= p[167];
  ret ^= p[8];
  ret ^= p[113];

/* quadratic taps */
  ret ^= p[31]&p[286];
  ret ^= p[162]|p[176];
  ret ^= p[235]|p[234];

  return(ret);
}


local int munge225(register int *p)
{
  register int ret;

  ret = 927069547;

/* linear taps */
  ret ^= p[144];
  ret -= p[73];
  ret += p[165];
  ret ^= p[287];
  ret -= p[120];
  ret ^= p[15];

/* quadratic taps */
  ret ^= p[9]&p[258];
  ret ^= p[27]&p[176];
  ret ^= p[41]|p[30];

  return(ret);
}


local int munge226(register int *p)
{
  register int ret;

  ret = 743504266;

/* linear taps */
  ret ^= p[251];
  ret ^= p[256];
  ret ^= p[275];
  ret += p[185];
  ret ^= p[229];
  ret -= p[192];

/* quadratic taps */
  ret ^= p[193]&p[153];
  ret += p[221]&p[280];
  ret ^= p[63]&p[22];

  return(ret);
}


local int munge227(register int *p)
{
  register int ret;

  ret = 1564523130;

/* linear taps */
  ret ^= p[75];
  ret += p[250];
  ret -= p[178];
  ret += p[131];
  ret += p[270];
  ret += p[56];

/* quadratic taps */
  ret ^= p[19]&p[174];
  ret += p[104]&p[33];
  ret ^= p[267]&p[258];

  return(ret);
}


local int munge228(register int *p)
{
  register int ret;

  ret = 1466086112;

/* linear taps */
  ret ^= p[243];
  ret -= p[112];
  ret ^= p[35];
  ret += p[97];
  ret += p[44];
  ret ^= p[63];

/* quadratic taps */
  ret += p[171]&p[82];
  ret += p[52]|p[211];
  ret ^= p[212]&p[194];

  return(ret);
}


local int munge229(register int *p)
{
  register int ret;

  ret = -891037342;

/* linear taps */
  ret += p[32];
  ret ^= p[202];
  ret ^= p[198];
  ret ^= p[37];
  ret ^= p[176];
  ret -= p[218];

/* quadratic taps */
  ret ^= p[145]&p[259];
  ret ^= p[125]|p[140];
  ret -= p[199]|p[167];

  return(ret);
}


local int munge230(register int *p)
{
  register int ret;

  ret = 1182252850;

/* linear taps */
  ret ^= p[77];
  ret -= p[247];
  ret -= p[92];
  ret += p[119];
  ret ^= p[219];
  ret ^= p[220];

/* quadratic taps */
  ret ^= p[71]|p[243];
  ret ^= p[253]&p[204];
  ret -= p[237]|p[45];

  return(ret);
}


local int munge231(register int *p)
{
  register int ret;

  ret = -1006575423;

/* linear taps */
  ret ^= p[75];
  ret ^= p[5];
  ret ^= p[52];
  ret += p[82];
  ret -= p[29];
  ret ^= p[126];

/* quadratic taps */
  ret ^= p[149]&p[35];
  ret ^= p[129]|p[172];
  ret -= p[19]|p[190];

  return(ret);
}


local int munge232(register int *p)
{
  register int ret;

  ret = 577779251;

/* linear taps */
  ret ^= p[49];
  ret ^= p[133];
  ret ^= p[34];
  ret ^= p[154];
  ret += p[105];
  ret ^= p[63];

/* quadratic taps */
  ret ^= p[38]&p[2];
  ret += p[18]|p[183];
  ret ^= p[42]|p[200];

  return(ret);
}


local int munge233(register int *p)
{
  register int ret;

  ret = -618150722;

/* linear taps */
  ret ^= p[158];
  ret += p[176];
  ret += p[163];
  ret ^= p[72];
  ret ^= p[186];
  ret ^= p[274];

/* quadratic taps */
  ret += p[131]|p[152];
  ret -= p[282]|p[187];
  ret += p[112]&p[32];

  return(ret);
}


local int munge234(register int *p)
{
  register int ret;

  ret = 1568213554;

/* linear taps */
  ret -= p[213];
  ret ^= p[160];
  ret ^= p[287];
  ret -= p[228];
  ret -= p[169];
  ret -= p[289];

/* quadratic taps */
  ret ^= p[25]&p[74];
  ret ^= p[212]&p[195];
  ret += p[260]|p[146];

  return(ret);
}


local int munge235(register int *p)
{
  register int ret;

  ret = -1678144185;

/* linear taps */
  ret -= p[197];
  ret ^= p[265];
  ret += p[180];
  ret += p[122];
  ret -= p[232];
  ret += p[115];

/* quadratic taps */
  ret ^= p[276]&p[126];
  ret += p[150]|p[142];
  ret += p[175]&p[181];

  return(ret);
}


local int munge236(register int *p)
{
  register int ret;

  ret = 1453876266;

/* linear taps */
  ret ^= p[180];
  ret ^= p[115];
  ret ^= p[122];
  ret ^= p[79];
  ret += p[65];
  ret ^= p[134];

/* quadratic taps */
  ret ^= p[38]|p[88];
  ret ^= p[212]|p[34];
  ret += p[253]&p[197];

  return(ret);
}


local int munge237(register int *p)
{
  register int ret;

  ret = -945617183;

/* linear taps */
  ret += p[204];
  ret ^= p[291];
  ret ^= p[159];
  ret += p[145];
  ret ^= p[226];
  ret ^= p[215];

/* quadratic taps */
  ret += p[243]&p[193];
  ret -= p[269]&p[45];
  ret ^= p[77]|p[42];

  return(ret);
}


local int munge238(register int *p)
{
  register int ret;

  ret = -1129596274;

/* linear taps */
  ret -= p[113];
  ret ^= p[21];
  ret ^= p[34];
  ret ^= p[46];
  ret -= p[94];
  ret ^= p[147];

/* quadratic taps */
  ret ^= p[194]|p[20];
  ret += p[118]&p[135];
  ret ^= p[141]&p[101];

  return(ret);
}


local int munge239(register int *p)
{
  register int ret;

  ret = -161315368;

/* linear taps */
  ret ^= p[57];
  ret += p[73];
  ret += p[58];
  ret += p[273];
  ret ^= p[180];
  ret ^= p[176];

/* quadratic taps */
  ret ^= p[110]|p[138];
  ret += p[292]&p[276];
  ret ^= p[256]|p[204];

  return(ret);
}


local int munge240(register int *p)
{
  register int ret;

  ret = 511247314;

/* linear taps */
  ret ^= p[136];
  ret ^= p[255];
  ret ^= p[263];
  ret ^= p[44];
  ret ^= p[147];
  ret ^= p[92];

/* quadratic taps */
  ret ^= p[285]|p[141];
  ret ^= p[165]&p[240];
  ret += p[143]|p[93];

  return(ret);
}


local int munge241(register int *p)
{
  register int ret;

  ret = 1378293315;

/* linear taps */
  ret += p[155];
  ret -= p[129];
  ret ^= p[80];
  ret += p[20];
  ret ^= p[18];
  ret ^= p[71];

/* quadratic taps */
  ret ^= p[43]|p[167];
  ret ^= p[252]|p[173];
  ret ^= p[228]|p[112];

  return(ret);
}


local int munge242(register int *p)
{
  register int ret;

  ret = 1541140511;

/* linear taps */
  ret ^= p[118];
  ret -= p[67];
  ret ^= p[181];
  ret ^= p[251];
  ret ^= p[87];
  ret += p[7];

/* quadratic taps */
  ret ^= p[4]|p[54];
  ret += p[90]&p[84];
  ret -= p[140]&p[282];

  return(ret);
}


local int munge243(register int *p)
{
  register int ret;

  ret = 949698091;

/* linear taps */
  ret += p[256];
  ret ^= p[199];
  ret ^= p[59];
  ret += p[286];
  ret += p[99];
  ret += p[94];

/* quadratic taps */
  ret += p[192]|p[167];
  ret ^= p[33]|p[49];
  ret ^= p[257]&p[205];

  return(ret);
}


local int munge244(register int *p)
{
  register int ret;

  ret = -891817162;

/* linear taps */
  ret += p[193];
  ret ^= p[24];
  ret += p[100];
  ret -= p[82];
  ret ^= p[187];
  ret ^= p[61];

/* quadratic taps */
  ret += p[267]&p[186];
  ret -= p[214]&p[219];
  ret ^= p[203]|p[39];

  return(ret);
}


local int munge245(register int *p)
{
  register int ret;

  ret = -198418509;

/* linear taps */
  ret ^= p[157];
  ret += p[116];
  ret ^= p[180];
  ret ^= p[174];
  ret += p[28];
  ret -= p[264];

/* quadratic taps */
  ret += p[189]|p[222];
  ret ^= p[71]&p[200];
  ret ^= p[283]|p[169];

  return(ret);
}


local int munge246(register int *p)
{
  register int ret;

  ret = 760048555;

/* linear taps */
  ret += p[268];
  ret ^= p[103];
  ret ^= p[138];
  ret ^= p[21];
  ret ^= p[255];
  ret ^= p[130];

/* quadratic taps */
  ret ^= p[190]&p[223];
  ret ^= p[212]&p[171];
  ret ^= p[228]|p[244];

  return(ret);
}


local int munge247(register int *p)
{
  register int ret;

  ret = 1668719658;

/* linear taps */
  ret ^= p[183];
  ret ^= p[26];
  ret ^= p[143];
  ret += p[160];
  ret ^= p[132];
  ret ^= p[129];

/* quadratic taps */
  ret -= p[171]&p[166];
  ret -= p[291]&p[241];
  ret += p[55]|p[266];

  return(ret);
}


local int munge248(register int *p)
{
  register int ret;

  ret = -1133285580;

/* linear taps */
  ret ^= p[1];
  ret ^= p[90];
  ret -= p[60];
  ret += p[136];
  ret += p[87];
  ret ^= p[241];

/* quadratic taps */
  ret -= p[228]&p[292];
  ret ^= p[179]|p[253];
  ret += p[63]|p[174];

  return(ret);
}


local int munge249(register int *p)
{
  register int ret;

  ret = -1062992737;

/* linear taps */
  ret += p[262];
  ret ^= p[102];
  ret -= p[89];
  ret -= p[53];
  ret ^= p[149];
  ret ^= p[93];

/* quadratic taps */
  ret ^= p[106]|p[190];
  ret += p[29]&p[23];
  ret -= p[1]&p[281];

  return(ret);
}


local int munge250(register int *p)
{
  register int ret;

  ret = -1354762792;

/* linear taps */
  ret ^= p[25];
  ret ^= p[11];
  ret += p[103];
  ret ^= p[133];
  ret ^= p[212];
  ret ^= p[267];

/* quadratic taps */
  ret += p[7]|p[194];
  ret ^= p[260]|p[130];
  ret ^= p[290]|p[107];

  return(ret);
}


local int munge251(register int *p)
{
  register int ret;

  ret = 102298393;

/* linear taps */
  ret ^= p[53];
  ret ^= p[208];
  ret += p[9];
  ret += p[165];
  ret ^= p[153];
  ret ^= p[215];

/* quadratic taps */
  ret += p[200]|p[126];
  ret ^= p[71]&p[281];
  ret ^= p[163]&p[39];

  return(ret);
}


local int munge252(register int *p)
{
  register int ret;

  ret = -628337817;

/* linear taps */
  ret ^= p[101];
  ret ^= p[81];
  ret -= p[102];
  ret += p[19];
  ret ^= p[250];
  ret ^= p[247];

/* quadratic taps */
  ret -= p[244]|p[134];
  ret -= p[162]|p[266];
  ret ^= p[106]&p[76];

  return(ret);
}


local int munge253(register int *p)
{
  register int ret;

  ret = 2029738098;

/* linear taps */
  ret ^= p[8];
  ret ^= p[228];
  ret ^= p[37];
  ret ^= p[206];
  ret ^= p[219];
  ret ^= p[102];

/* quadratic taps */
  ret ^= p[212]|p[280];
  ret += p[107]|p[158];
  ret -= p[170]|p[119];

  return(ret);
}


local int munge254(register int *p)
{
  register int ret;

  ret = -847694489;

/* linear taps */
  ret ^= p[199];
  ret ^= p[48];
  ret ^= p[152];
  ret -= p[170];
  ret += p[26];
  ret -= p[111];

/* quadratic taps */
  ret ^= p[81]|p[6];
  ret += p[3]|p[284];
  ret -= p[238]|p[254];

  return(ret);
}


local int munge255(register int *p)
{
  register int ret;

  ret = -1240654646;

/* linear taps */
  ret += p[163];
  ret ^= p[103];
  ret ^= p[19];
  ret += p[180];
  ret -= p[112];
  ret ^= p[30];

/* quadratic taps */
  ret ^= p[57]|p[1];
  ret ^= p[167]|p[88];
  ret ^= p[70]|p[120];

  return(ret);
}


local int munge256(register int *p)
{
  register int ret;

  ret = -160406214;

/* linear taps */
  ret ^= p[266];
  ret -= p[168];
  ret ^= p[93];
  ret ^= p[251];
  ret ^= p[90];
  ret ^= p[39];

/* quadratic taps */
  ret ^= p[263]|p[256];
  ret ^= p[212]|p[214];
  ret += p[220]&p[242];

  return(ret);
}


local int munge257(register int *p)
{
  register int ret;

  ret = -1919643260;

/* linear taps */
  ret ^= p[291];
  ret -= p[62];
  ret ^= p[130];
  ret ^= p[241];
  ret ^= p[275];
  ret ^= p[164];

/* quadratic taps */
  ret -= p[7]|p[132];
  ret ^= p[101]|p[289];
  ret += p[247]&p[206];

  return(ret);
}


local int munge258(register int *p)
{
  register int ret;

  ret = -945937445;

/* linear taps */
  ret -= p[153];
  ret ^= p[251];
  ret ^= p[185];
  ret ^= p[291];
  ret -= p[52];
  ret ^= p[260];

/* quadratic taps */
  ret += p[164]|p[67];
  ret ^= p[38]|p[139];
  ret += p[249]|p[276];

  return(ret);
}


local int munge259(register int *p)
{
  register int ret;

  ret = -427725627;

/* linear taps */
  ret ^= p[123];
  ret ^= p[234];
  ret ^= p[107];
  ret += p[156];
  ret -= p[86];
  ret ^= p[84];

/* quadratic taps */
  ret ^= p[286]&p[29];
  ret -= p[283]&p[87];
  ret += p[231]&p[158];

  return(ret);
}


local int munge260(register int *p)
{
  register int ret;

  ret = -1327173779;

/* linear taps */
  ret ^= p[135];
  ret -= p[144];
  ret -= p[193];
  ret ^= p[200];
  ret -= p[176];
  ret += p[120];

/* quadratic taps */
  ret ^= p[100]|p[26];
  ret -= p[124]&p[223];
  ret ^= p[291]&p[197];

  return(ret);
}


local int munge261(register int *p)
{
  register int ret;

  ret = 695149160;

/* linear taps */
  ret ^= p[77];
  ret -= p[184];
  ret -= p[238];
  ret -= p[172];
  ret ^= p[108];
  ret -= p[134];

/* quadratic taps */
  ret += p[138]&p[39];
  ret += p[286]|p[173];
  ret += p[285]&p[175];

  return(ret);
}


local int munge262(register int *p)
{
  register int ret;

  ret = 1026633939;

/* linear taps */
  ret -= p[283];
  ret -= p[216];
  ret ^= p[36];
  ret -= p[101];
  ret ^= p[89];
  ret -= p[169];

/* quadratic taps */
  ret ^= p[14]&p[146];
  ret ^= p[239]|p[162];
  ret ^= p[207]&p[8];

  return(ret);
}


local int munge263(register int *p)
{
  register int ret;

  ret = 2026196594;

/* linear taps */
  ret += p[230];
  ret ^= p[102];
  ret += p[62];
  ret ^= p[11];
  ret ^= p[45];
  ret -= p[76];

/* quadratic taps */
  ret += p[117]|p[275];
  ret ^= p[138]|p[191];
  ret -= p[243]&p[55];

  return(ret);
}


local int munge264(register int *p)
{
  register int ret;

  ret = 1257420124;

/* linear taps */
  ret += p[163];
  ret ^= p[40];
  ret += p[110];
  ret ^= p[96];
  ret += p[234];
  ret ^= p[278];

/* quadratic taps */
  ret += p[222]&p[94];
  ret ^= p[45]|p[270];
  ret ^= p[175]|p[84];

  return(ret);
}


local int munge265(register int *p)
{
  register int ret;

  ret = 1479448152;

/* linear taps */
  ret ^= p[114];
  ret -= p[101];
  ret ^= p[45];
  ret -= p[144];
  ret ^= p[198];
  ret ^= p[161];

/* quadratic taps */
  ret += p[94]&p[179];
  ret ^= p[89]&p[160];
  ret += p[129]|p[61];

  return(ret);
}


local int munge266(register int *p)
{
  register int ret;

  ret = 1197053047;

/* linear taps */
  ret += p[185];
  ret ^= p[197];
  ret -= p[56];
  ret ^= p[14];
  ret ^= p[262];
  ret ^= p[251];

/* quadratic taps */
  ret ^= p[48]|p[45];
  ret -= p[179]&p[115];
  ret ^= p[277]&p[260];

  return(ret);
}


local int munge267(register int *p)
{
  register int ret;

  ret = -1142709772;

/* linear taps */
  ret += p[141];
  ret -= p[198];
  ret -= p[260];
  ret -= p[139];
  ret ^= p[279];
  ret -= p[36];

/* quadratic taps */
  ret ^= p[232]|p[122];
  ret += p[120]|p[140];
  ret += p[149]|p[195];

  return(ret);
}


local int munge268(register int *p)
{
  register int ret;

  ret = 664727902;

/* linear taps */
  ret += p[222];
  ret ^= p[4];
  ret -= p[8];
  ret ^= p[160];
  ret ^= p[6];
  ret ^= p[219];

/* quadratic taps */
  ret -= p[69]|p[186];
  ret -= p[175]|p[172];
  ret += p[192]|p[248];

  return(ret);
}


local int munge269(register int *p)
{
  register int ret;

  ret = -599537008;

/* linear taps */
  ret -= p[81];
  ret += p[256];
  ret ^= p[289];
  ret ^= p[129];
  ret ^= p[114];
  ret += p[178];

/* quadratic taps */
  ret ^= p[260]|p[162];
  ret ^= p[104]|p[22];
  ret ^= p[218]&p[148];

  return(ret);
}


local int munge270(register int *p)
{
  register int ret;

  ret = 685465256;

/* linear taps */
  ret += p[39];
  ret ^= p[20];
  ret += p[258];
  ret ^= p[142];
  ret ^= p[254];
  ret ^= p[18];

/* quadratic taps */
  ret ^= p[46]|p[35];
  ret -= p[128]|p[178];
  ret ^= p[27]&p[82];

  return(ret);
}


local int munge271(register int *p)
{
  register int ret;

  ret = -1034089655;

/* linear taps */
  ret ^= p[2];
  ret ^= p[90];
  ret -= p[32];
  ret ^= p[163];
  ret ^= p[200];
  ret += p[78];

/* quadratic taps */
  ret ^= p[83]&p[146];
  ret -= p[53]|p[22];
  ret -= p[103]|p[124];

  return(ret);
}


local int munge272(register int *p)
{
  register int ret;

  ret = 825631819;

/* linear taps */
  ret -= p[15];
  ret ^= p[1];
  ret -= p[138];
  ret ^= p[55];
  ret ^= p[282];
  ret += p[82];

/* quadratic taps */
  ret ^= p[121]&p[41];
  ret += p[208]&p[105];
  ret ^= p[9]&p[192];

  return(ret);
}


local int munge273(register int *p)
{
  register int ret;

  ret = -1666029363;

/* linear taps */
  ret += p[54];
  ret ^= p[195];
  ret ^= p[264];
  ret ^= p[121];
  ret ^= p[238];
  ret -= p[61];

/* quadratic taps */
  ret -= p[25]|p[123];
  ret ^= p[258]|p[270];
  ret ^= p[8]&p[144];

  return(ret);
}


local int munge274(register int *p)
{
  register int ret;

  ret = 982449917;

/* linear taps */
  ret += p[186];
  ret -= p[74];
  ret -= p[253];
  ret -= p[259];
  ret -= p[10];
  ret ^= p[160];

/* quadratic taps */
  ret += p[83]&p[245];
  ret ^= p[77]&p[102];
  ret ^= p[146]&p[9];

  return(ret);
}


local int munge275(register int *p)
{
  register int ret;

  ret = -840385484;

/* linear taps */
  ret ^= p[56];
  ret -= p[105];
  ret ^= p[193];
  ret ^= p[246];
  ret ^= p[118];
  ret ^= p[198];

/* quadratic taps */
  ret ^= p[132]&p[210];
  ret += p[63]&p[218];
  ret += p[92]&p[86];

  return(ret);
}


local int munge276(register int *p)
{
  register int ret;

  ret = 342026388;

/* linear taps */
  ret ^= p[184];
  ret += p[247];
  ret ^= p[196];
  ret ^= p[180];
  ret ^= p[174];
  ret += p[155];

/* quadratic taps */
  ret ^= p[239]&p[276];
  ret += p[37]&p[134];
  ret ^= p[67]|p[103];

  return(ret);
}


local int munge277(register int *p)
{
  register int ret;

  ret = -1287077956;

/* linear taps */
  ret -= p[267];
  ret -= p[151];
  ret ^= p[42];
  ret += p[41];
  ret ^= p[49];
  ret += p[137];

/* quadratic taps */
  ret ^= p[38]&p[67];
  ret ^= p[175]&p[268];
  ret += p[168]|p[222];

  return(ret);
}


local int munge278(register int *p)
{
  register int ret;

  ret = 752023426;

/* linear taps */
  ret += p[275];
  ret ^= p[39];
  ret -= p[96];
  ret ^= p[216];
  ret -= p[204];
  ret -= p[20];

/* quadratic taps */
  ret -= p[201]&p[239];
  ret += p[230]|p[162];
  ret ^= p[248]&p[48];

  return(ret);
}


local int munge279(register int *p)
{
  register int ret;

  ret = -43762451;

/* linear taps */
  ret ^= p[33];
  ret += p[43];
  ret ^= p[3];
  ret -= p[267];
  ret ^= p[75];
  ret ^= p[77];

/* quadratic taps */
  ret += p[139]|p[189];
  ret ^= p[169]&p[129];
  ret ^= p[141]&p[262];

  return(ret);
}


local int munge280(register int *p)
{
  register int ret;

  ret = 973059642;

/* linear taps */
  ret ^= p[176];
  ret += p[255];
  ret -= p[132];
  ret ^= p[55];
  ret ^= p[225];
  ret += p[45];

/* quadratic taps */
  ret += p[50]|p[175];
  ret += p[4]&p[197];
  ret ^= p[247]&p[67];

  return(ret);
}


local int munge281(register int *p)
{
  register int ret;

  ret = -1648919006;

/* linear taps */
  ret ^= p[67];
  ret ^= p[195];
  ret ^= p[257];
  ret += p[89];
  ret -= p[14];
  ret ^= p[243];

/* quadratic taps */
  ret -= p[281]&p[283];
  ret += p[144]|p[180];
  ret += p[290]|p[17];

  return(ret);
}


local int munge282(register int *p)
{
  register int ret;

  ret = 894419580;

/* linear taps */
  ret ^= p[114];
  ret ^= p[4];
  ret -= p[171];
  ret += p[154];
  ret -= p[126];
  ret ^= p[118];

/* quadratic taps */
  ret += p[199]&p[109];
  ret += p[193]&p[34];
  ret -= p[153]|p[13];

  return(ret);
}


local int (*(munge_array[283]))(int *p) = 
{
  &munge0,
  &munge1,
  &munge2,
  &munge3,
  &munge4,
  &munge5,
  &munge6,
  &munge7,
  &munge8,
  &munge9,
  &munge10,
  &munge11,
  &munge12,
  &munge13,
  &munge14,
  &munge15,
  &munge16,
  &munge17,
  &munge18,
  &munge19,
  &munge20,
  &munge21,
  &munge22,
  &munge23,
  &munge24,
  &munge25,
  &munge26,
  &munge27,
  &munge28,
  &munge29,
  &munge30,
  &munge31,
  &munge32,
  &munge33,
  &munge34,
  &munge35,
  &munge36,
  &munge37,
  &munge38,
  &munge39,
  &munge40,
  &munge41,
  &munge42,
  &munge43,
  &munge44,
  &munge45,
  &munge46,
  &munge47,
  &munge48,
  &munge49,
  &munge50,
  &munge51,
  &munge52,
  &munge53,
  &munge54,
  &munge55,
  &munge56,
  &munge57,
  &munge58,
  &munge59,
  &munge60,
  &munge61,
  &munge62,
  &munge63,
  &munge64,
  &munge65,
  &munge66,
  &munge67,
  &munge68,
  &munge69,
  &munge70,
  &munge71,
  &munge72,
  &munge73,
  &munge74,
  &munge75,
  &munge76,
  &munge77,
  &munge78,
  &munge79,
  &munge80,
  &munge81,
  &munge82,
  &munge83,
  &munge84,
  &munge85,
  &munge86,
  &munge87,
  &munge88,
  &munge89,
  &munge90,
  &munge91,
  &munge92,
  &munge93,
  &munge94,
  &munge95,
  &munge96,
  &munge97,
  &munge98,
  &munge99,
  &munge100,
  &munge101,
  &munge102,
  &munge103,
  &munge104,
  &munge105,
  &munge106,
  &munge107,
  &munge108,
  &munge109,
  &munge110,
  &munge111,
  &munge112,
  &munge113,
  &munge114,
  &munge115,
  &munge116,
  &munge117,
  &munge118,
  &munge119,
  &munge120,
  &munge121,
  &munge122,
  &munge123,
  &munge124,
  &munge125,
  &munge126,
  &munge127,
  &munge128,
  &munge129,
  &munge130,
  &munge131,
  &munge132,
  &munge133,
  &munge134,
  &munge135,
  &munge136,
  &munge137,
  &munge138,
  &munge139,
  &munge140,
  &munge141,
  &munge142,
  &munge143,
  &munge144,
  &munge145,
  &munge146,
  &munge147,
  &munge148,
  &munge149,
  &munge150,
  &munge151,
  &munge152,
  &munge153,
  &munge154,
  &munge155,
  &munge156,
  &munge157,
  &munge158,
  &munge159,
  &munge160,
  &munge161,
  &munge162,
  &munge163,
  &munge164,
  &munge165,
  &munge166,
  &munge167,
  &munge168,
  &munge169,
  &munge170,
  &munge171,
  &munge172,
  &munge173,
  &munge174,
  &munge175,
  &munge176,
  &munge177,
  &munge178,
  &munge179,
  &munge180,
  &munge181,
  &munge182,
  &munge183,
  &munge184,
  &munge185,
  &munge186,
  &munge187,
  &munge188,
  &munge189,
  &munge190,
  &munge191,
  &munge192,
  &munge193,
  &munge194,
  &munge195,
  &munge196,
  &munge197,
  &munge198,
  &munge199,
  &munge200,
  &munge201,
  &munge202,
  &munge203,
  &munge204,
  &munge205,
  &munge206,
  &munge207,
  &munge208,
  &munge209,
  &munge210,
  &munge211,
  &munge212,
  &munge213,
  &munge214,
  &munge215,
  &munge216,
  &munge217,
  &munge218,
  &munge219,
  &munge220,
  &munge221,
  &munge222,
  &munge223,
  &munge224,
  &munge225,
  &munge226,
  &munge227,
  &munge228,
  &munge229,
  &munge230,
  &munge231,
  &munge232,
  &munge233,
  &munge234,
  &munge235,
  &munge236,
  &munge237,
  &munge238,
  &munge239,
  &munge240,
  &munge241,
  &munge242,
  &munge243,
  &munge244,
  &munge245,
  &munge246,
  &munge247,
  &munge248,
  &munge249,
  &munge250,
  &munge251,
  &munge252,
  &munge253,
  &munge254,
  &munge255,
  &munge256,
  &munge257,
  &munge258,
  &munge259,
  &munge260,
  &munge261,
  &munge262,
  &munge263,
  &munge264,
  &munge265,
  &munge266,
  &munge267,
  &munge268,
  &munge269,
  &munge270,
  &munge271,
  &munge272,
  &munge273,
  &munge274,
  &munge275,
  &munge276,
  &munge277,
  &munge278,
  &munge279,
  &munge280,
  &munge281,
  &munge282
};


local int hash_int(int i)
{
  char *pi;
  int count,ret;

  ret = 777777;

  pi = (char *)(&i);

  for(count=0;count<(int)sizeof(int);++count)
  {
    ret ^= wn_random_int_table[2000 + pi[count]];
  }

  return(ret);
}


local void print_state()
{
  int i;

  printf("pointer = %d\n",pointer);
  printf("munge_pointer = %d\n",munge_pointer);
  printf("{");

  for(i=0;i<BUFFER_SIZE;++i)
  {
    if(i > 0)
    {
      printf(",");
    }

    if(i%6 == 5)
    {
      printf("\n");
    }

    printf("0x%lx",state[i]);
  }

  printf("}\n");
}


void wn_seed_random_number_generator(int seed)
{
  int i;

  wn_memcpy(state,wn_random_int_table+WN_RANDOM_INT_TABLE_SIZE-BUFFER_SIZE,
      BUFFER_SIZE*sizeof(int));

  for(i=0;i<BUFFER_SIZE;++i)
  {
    state[i] ^= hash_int(i+seed);
  }

  pointer = WINDOW_SIZE;
  munge_pointer = 0;
  munge_pointer_out = OUT_OFFSET;
  number_of_saved_bits = 0;

  for(i=0;i<7*WINDOW_SIZE;++i)
  {
    (void)wn_random_int();
  }

  /*
  print_state();
  */
}


#define CHAR_MASK  ((1<<8)-1)


int wn_random_int(void)
{
  int ret,feedback_bit;
  int *p;

  p = &(state[pointer]);

  feedback_bit = (*(munge_array[munge_pointer]))(p);

  ++munge_pointer;
  if(munge_pointer >= NUM_MUNGE_FUNCS)
  {
    munge_pointer = 0;

    feedback_bit ^= wn_random_int_table[
         (
           260 
             +
           (
             (
         (p[97]>>(3*8))
           ^
         (
           (p[123]>>(0*8))
             |
           (p[125]>>(1*8))
                                 )
           ^
         (
           (p[220]>>(1*8))
             &
           (p[238]>>(2*8))
         )
             )&CHAR_MASK
           )
         )&CHAR_MASK];
  }

  *p ^= feedback_bit;

  ret = (*(munge_array[munge_pointer_out]))(p);

  ++munge_pointer_out;
  if(munge_pointer_out >= NUM_MUNGE_FUNCS)
  {
    munge_pointer_out = 0;
  }

  --pointer;
  if(pointer < 0)
  {
    wn_memcpy(&(state[BUFFER_SIZE-WINDOW_SIZE]),state,WINDOW_SIZE*sizeof(int));
    pointer = BUFFER_SIZE-WINDOW_SIZE;
  }

  return(ret);
}


#if 0
int wn_random_int(void)
{
  int ret,feedback_bit;
  int *p;
  char *pc;

  p = &(state[pointer]);

  feedback_bit = (*(munge_array[munge_pointer]))(p);

  ++munge_pointer;
  if(munge_pointer >= NUM_MUNGE_FUNCS)
  {
    munge_pointer = 0;

    pc = (char *)p;
    feedback_bit ^= wn_random_int_table[260 +
             (
               pc[4*97+0]
           ^
         (pc[4*123+3]|pc[4*125+2])
           ^
         (pc[4*220+2]&pc[4*238+1])
             )&CHAR_MASK];
  }

  *p ^= feedback_bit;

  ret = (*(munge_array[munge_pointer_out]))(p);

  ++munge_pointer_out;
  if(munge_pointer_out >= NUM_MUNGE_FUNCS)
  {
    munge_pointer_out = 0;
  }

  --pointer;
  if(pointer < 0)
  {
    wn_memcpy(&(state[BUFFER_SIZE-WINDOW_SIZE]),state,WINDOW_SIZE*sizeof(int));
    pointer = BUFFER_SIZE-WINDOW_SIZE;
  }

  return(ret);
}
#endif


#if 0
int wn_random_int(void)
{
  return(wn_true_random_int());
}


int wn_random_n_bits(int n)
{
  int ret;

  if(number_of_saved_bits < n)
  {
    saved_bits = wn_random_int();
    number_of_saved_bits = 32;
  }

  ret = saved_bits&((1<<n)-1);
  saved_bits >>= n;
  number_of_saved_bits -= n;

  return(ret);
}
#endif


int wn_random_n_bits(int n)
{
  int ret;

  if(n <= number_of_saved_bits)
  {
    ret = saved_bits&((1<<n)-1);
    saved_bits >>= n;
    number_of_saved_bits -= n;

    return(ret);
  }
  else /* n > number_of_saved_bits */
  {
    int bit_deficit;

    bit_deficit = n - number_of_saved_bits;

    ret = saved_bits&((1<<number_of_saved_bits)-1);
    ret <<= bit_deficit;

    saved_bits = wn_random_int();
    ret |= ( saved_bits&((1<<bit_deficit)-1) );
    saved_bits >>= bit_deficit;
    number_of_saved_bits = 32 - bit_deficit;

    return(ret);
  }
}


int wn_random_bit(void)
{
  int ret;

  if(number_of_saved_bits < 1)
  {
    saved_bits = wn_random_int();
    number_of_saved_bits = 32;
  }

  ret = (int)(saved_bits&1);
  saved_bits >>= 1;
  --number_of_saved_bits;

  return(ret);
}

