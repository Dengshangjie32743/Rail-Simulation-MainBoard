/**
 * @brief CRC计算
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author HARDWARE group
 * @file   plt_CalculateCrc.c
 * @ingroup SLib
 *
 * @date 2021-11-19
 *
 */

#include "TypeInc.h"
/* begin modified by deng 2023.10.27 */
#include "simPro.h"
#include "SLib.h"
#include "ucos_ii.h"

/* end modified by deng 2023.10.27 */

/** CRC32码表(多项式0x04c11db7，初始值为0xffffffff,结果异或系数为0x00000000) */
uint32_t g_ulSLibVoteCrc32Table[256] =
{
    0x00000000U, 0x77073096U, 0xEE0E612CU, 0x990951BAU, 0x076DC419U, 0x706AF48FU, 0xE963A535U, 0x9E6495A3U,
    0x0EDB8832U, 0x79DCB8A4U, 0xE0D5E91EU, 0x97D2D988U, 0x09B64C2BU, 0x7EB17CBDU, 0xE7B82D07U, 0x90BF1D91U,
    0x1DB71064U, 0x6AB020F2U, 0xF3B97148U, 0x84BE41DEU, 0x1ADAD47DU, 0x6DDDE4EBU, 0xF4D4B551U, 0x83D385C7U,
    0x136C9856U, 0x646BA8C0U, 0xFD62F97AU, 0x8A65C9ECU, 0x14015C4FU, 0x63066CD9U, 0xFA0F3D63U, 0x8D080DF5U,
    0x3B6E20C8U, 0x4C69105EU, 0xD56041E4U, 0xA2677172U, 0x3C03E4D1U, 0x4B04D447U, 0xD20D85FDU, 0xA50AB56BU,
    0x35B5A8FAU, 0x42B2986CU, 0xDBBBC9D6U, 0xACBCF940U, 0x32D86CE3U, 0x45DF5C75U, 0xDCD60DCFU, 0xABD13D59U,
    0x26D930ACU, 0x51DE003AU, 0xC8D75180U, 0xBFD06116U, 0x21B4F4B5U, 0x56B3C423U, 0xCFBA9599U, 0xB8BDA50FU,
    0x2802B89EU, 0x5F058808U, 0xC60CD9B2U, 0xB10BE924U, 0x2F6F7C87U, 0x58684C11U, 0xC1611DABU, 0xB6662D3DU,
    0x76DC4190U, 0x01DB7106U, 0x98D220BCU, 0xEFD5102AU, 0x71B18589U, 0x06B6B51FU, 0x9FBFE4A5U, 0xE8B8D433U,
    0x7807C9A2U, 0x0F00F934U, 0x9609A88EU, 0xE10E9818U, 0x7F6A0DBBU, 0x086D3D2DU, 0x91646C97U, 0xE6635C01U,
    0x6B6B51F4U, 0x1C6C6162U, 0x856530D8U, 0xF262004EU, 0x6C0695EDU, 0x1B01A57BU, 0x8208F4C1U, 0xF50FC457U,
    0x65B0D9C6U, 0x12B7E950U, 0x8BBEB8EAU, 0xFCB9887CU, 0x62DD1DDFU, 0x15DA2D49U, 0x8CD37CF3U, 0xFBD44C65U,
    0x4DB26158U, 0x3AB551CEU, 0xA3BC0074U, 0xD4BB30E2U, 0x4ADFA541U, 0x3DD895D7U, 0xA4D1C46DU, 0xD3D6F4FBU,
    0x4369E96AU, 0x346ED9FCU, 0xAD678846U, 0xDA60B8D0U, 0x44042D73U, 0x33031DE5U, 0xAA0A4C5FU, 0xDD0D7CC9U,
    0x5005713CU, 0x270241AAU, 0xBE0B1010U, 0xC90C2086U, 0x5768B525U, 0x206F85B3U, 0xB966D409U, 0xCE61E49FU,
    0x5EDEF90EU, 0x29D9C998U, 0xB0D09822U, 0xC7D7A8B4U, 0x59B33D17U, 0x2EB40D81U, 0xB7BD5C3BU, 0xC0BA6CADU,
    0xEDB88320U, 0x9ABFB3B6U, 0x03B6E20CU, 0x74B1D29AU, 0xEAD54739U, 0x9DD277AFU, 0x04DB2615U, 0x73DC1683U,
    0xE3630B12U, 0x94643B84U, 0x0D6D6A3EU, 0x7A6A5AA8U, 0xE40ECF0BU, 0x9309FF9DU, 0x0A00AE27U, 0x7D079EB1U,
    0xF00F9344U, 0x8708A3D2U, 0x1E01F268U, 0x6906C2FEU, 0xF762575DU, 0x806567CBU, 0x196C3671U, 0x6E6B06E7U,
    0xFED41B76U, 0x89D32BE0U, 0x10DA7A5AU, 0x67DD4ACCU, 0xF9B9DF6FU, 0x8EBEEFF9U, 0x17B7BE43U, 0x60B08ED5U,
    0xD6D6A3E8U, 0xA1D1937EU, 0x38D8C2C4U, 0x4FDFF252U, 0xD1BB67F1U, 0xA6BC5767U, 0x3FB506DDU, 0x48B2364BU,
    0xD80D2BDAU, 0xAF0A1B4CU, 0x36034AF6U, 0x41047A60U, 0xDF60EFC3U, 0xA867DF55U, 0x316E8EEFU, 0x4669BE79U,
    0xCB61B38CU, 0xBC66831AU, 0x256FD2A0U, 0x5268E236U, 0xCC0C7795U, 0xBB0B4703U, 0x220216B9U, 0x5505262FU,
    0xC5BA3BBEU, 0xB2BD0B28U, 0x2BB45A92U, 0x5CB36A04U, 0xC2D7FFA7U, 0xB5D0CF31U, 0x2CD99E8BU, 0x5BDEAE1DU,
    0x9B64C2B0U, 0xEC63F226U, 0x756AA39CU, 0x026D930AU, 0x9C0906A9U, 0xEB0E363FU, 0x72076785U, 0x05005713U,
    0x95BF4A82U, 0xE2B87A14U, 0x7BB12BAEU, 0x0CB61B38U, 0x92D28E9BU, 0xE5D5BE0DU, 0x7CDCEFB7U, 0x0BDBDF21U,
    0x86D3D2D4U, 0xF1D4E242U, 0x68DDB3F8U, 0x1FDA836EU, 0x81BE16CDU, 0xF6B9265BU, 0x6FB077E1U, 0x18B74777U,
    0x88085AE6U, 0xFF0F6A70U, 0x66063BCAU, 0x11010B5CU, 0x8F659EFFU, 0xF862AE69U, 0x616BFFD3U, 0x166CCF45U,
    0xA00AE278U, 0xD70DD2EEU, 0x4E048354U, 0x3903B3C2U, 0xA7672661U, 0xD06016F7U, 0x4969474DU, 0x3E6E77DBU,
    0xAED16A4AU, 0xD9D65ADCU, 0x40DF0B66U, 0x37D83BF0U, 0xA9BCAE53U, 0xDEBB9EC5U, 0x47B2CF7FU, 0x30B5FFE9U,
    0xBDBDF21CU, 0xCABAC28AU, 0x53B39330U, 0x24B4A3A6U, 0xBAD03605U, 0xCDD70693U, 0x54DE5729U, 0x23D967BFU,
    0xB3667A2EU, 0xC4614AB8U, 0x5D681B02U, 0x2A6F2B94U, 0xB40BBE37U, 0xC30C8EA1U, 0x5A05DF1BU, 0x2D02EF8DU
 };

/** CRC32_C码表(多项式0x1EDC6F41，初始值为0xFFFFFFFF,结果异或系数为0x00000000),输入反转，输出反转 */
uint32_t g_ulSLibCrc32CTable[256] =
{
    0x00000000U, 0xF26B8303U, 0xE13B70F7U, 0x1350F3F4U, 0xC79A971FU, 0x35F1141CU, 0x26A1E7E8U, 0xD4CA64EBU,
    0x8AD958CFU, 0x78B2DBCCU, 0x6BE22838U, 0x9989AB3BU, 0x4D43CFD0U, 0xBF284CD3U, 0xAC78BF27U, 0x5E133C24U,
    0x105EC76FU, 0xE235446CU, 0xF165B798U, 0x030E349BU, 0xD7C45070U, 0x25AFD373U, 0x36FF2087U, 0xC494A384U,
    0x9A879FA0U, 0x68EC1CA3U, 0x7BBCEF57U, 0x89D76C54U, 0x5D1D08BFU, 0xAF768BBCU, 0xBC267848U, 0x4E4DFB4BU,
    0x20BD8EDEU, 0xD2D60DDDU, 0xC186FE29U, 0x33ED7D2AU, 0xE72719C1U, 0x154C9AC2U, 0x061C6936U, 0xF477EA35U,
    0xAA64D611U, 0x580F5512U, 0x4B5FA6E6U, 0xB93425E5U, 0x6DFE410EU, 0x9F95C20DU, 0x8CC531F9U, 0x7EAEB2FAU,
    0x30E349B1U, 0xC288CAB2U, 0xD1D83946U, 0x23B3BA45U, 0xF779DEAEU, 0x05125DADU, 0x1642AE59U, 0xE4292D5AU,
    0xBA3A117EU, 0x4851927DU, 0x5B016189U, 0xA96AE28AU, 0x7DA08661U, 0x8FCB0562U, 0x9C9BF696U, 0x6EF07595U,
    0x417B1DBCU, 0xB3109EBFU, 0xA0406D4BU, 0x522BEE48U, 0x86E18AA3U, 0x748A09A0U, 0x67DAFA54U, 0x95B17957U,
    0xCBA24573U, 0x39C9C670U, 0x2A993584U, 0xD8F2B687U, 0x0C38D26CU, 0xFE53516FU, 0xED03A29BU, 0x1F682198U,
    0x5125DAD3U, 0xA34E59D0U, 0xB01EAA24U, 0x42752927U, 0x96BF4DCCU, 0x64D4CECFU, 0x77843D3BU, 0x85EFBE38U,
    0xDBFC821CU, 0x2997011FU, 0x3AC7F2EBU, 0xC8AC71E8U, 0x1C661503U, 0xEE0D9600U, 0xFD5D65F4U, 0x0F36E6F7U,
    0x61C69362U, 0x93AD1061U, 0x80FDE395U, 0x72966096U, 0xA65C047DU, 0x5437877EU, 0x4767748AU, 0xB50CF789U,
    0xEB1FCBADU, 0x197448AEU, 0x0A24BB5AU, 0xF84F3859U, 0x2C855CB2U, 0xDEEEDFB1U, 0xCDBE2C45U, 0x3FD5AF46U,
    0x7198540DU, 0x83F3D70EU, 0x90A324FAU, 0x62C8A7F9U, 0xB602C312U, 0x44694011U, 0x5739B3E5U, 0xA55230E6U,
    0xFB410CC2U, 0x092A8FC1U, 0x1A7A7C35U, 0xE811FF36U, 0x3CDB9BDDU, 0xCEB018DEU, 0xDDE0EB2AU, 0x2F8B6829U,
    0x82F63B78U, 0x709DB87BU, 0x63CD4B8FU, 0x91A6C88CU, 0x456CAC67U, 0xB7072F64U, 0xA457DC90U, 0x563C5F93U,
    0x082F63B7U, 0xFA44E0B4U, 0xE9141340U, 0x1B7F9043U, 0xCFB5F4A8U, 0x3DDE77ABU, 0x2E8E845FU, 0xDCE5075CU,
    0x92A8FC17U, 0x60C37F14U, 0x73938CE0U, 0x81F80FE3U, 0x55326B08U, 0xA759E80BU, 0xB4091BFFU, 0x466298FCU,
    0x1871A4D8U, 0xEA1A27DBU, 0xF94AD42FU, 0x0B21572CU, 0xDFEB33C7U, 0x2D80B0C4U, 0x3ED04330U, 0xCCBBC033U,
    0xA24BB5A6U, 0x502036A5U, 0x4370C551U, 0xB11B4652U, 0x65D122B9U, 0x97BAA1BAU, 0x84EA524EU, 0x7681D14DU,
    0x2892ED69U, 0xDAF96E6AU, 0xC9A99D9EU, 0x3BC21E9DU, 0xEF087A76U, 0x1D63F975U, 0x0E330A81U, 0xFC588982U,
    0xB21572C9U, 0x407EF1CAU, 0x532E023EU, 0xA145813DU, 0x758FE5D6U, 0x87E466D5U, 0x94B49521U, 0x66DF1622U,
    0x38CC2A06U, 0xCAA7A905U, 0xD9F75AF1U, 0x2B9CD9F2U, 0xFF56BD19U, 0x0D3D3E1AU, 0x1E6DCDEEU, 0xEC064EEDU,
    0xC38D26C4U, 0x31E6A5C7U, 0x22B65633U, 0xD0DDD530U, 0x0417B1DBU, 0xF67C32D8U, 0xE52CC12CU, 0x1747422FU,
    0x49547E0BU, 0xBB3FFD08U, 0xA86F0EFCU, 0x5A048DFFU, 0x8ECEE914U, 0x7CA56A17U, 0x6FF599E3U, 0x9D9E1AE0U,
    0xD3D3E1ABU, 0x21B862A8U, 0x32E8915CU, 0xC083125FU, 0x144976B4U, 0xE622F5B7U, 0xF5720643U, 0x07198540U,
    0x590AB964U, 0xAB613A67U, 0xB831C993U, 0x4A5A4A90U, 0x9E902E7BU, 0x6CFBAD78U, 0x7FAB5E8CU, 0x8DC0DD8FU,
    0xE330A81AU, 0x115B2B19U, 0x020BD8EDU, 0xF0605BEEU, 0x24AA3F05U, 0xD6C1BC06U, 0xC5914FF2U, 0x37FACCF1U,
    0x69E9F0D5U, 0x9B8273D6U, 0x88D28022U, 0x7AB90321U, 0xAE7367CAU, 0x5C18E4C9U, 0x4F48173DU, 0xBD23943EU,
    0xF36E6F75U, 0x0105EC76U, 0x12551F82U, 0xE03E9C81U, 0x34F4F86AU, 0xC69F7B69U, 0xD5CF889DU, 0x27A40B9EU,
    0x79B737BAU, 0x8BDCB4B9U, 0x988C474DU, 0x6AE7C44EU, 0xBE2DA0A5U, 0x4C4623A6U, 0x5F16D052U, 0xAD7D5351U
};

/**
* @brief 连续计算表决crc32数值(多项式0x04c11db7，初始值为0xffffffff，结果异或系数为0x00000000，输入反转，输出反转)
* - [iSCP-SLib-SwCD-F-1803]
*
* @param[in] initvalue 初始值
* @param[in] pdata     数据指针
* @param[in] datalen   数据长度
*
* @retval crc crc计算结果
* @see [iSCP-MPU-SwAD-F-0301][iSCP-ATO-SwAD-F-0013]
*/
uint32_t S_ContinueVoteCrc32(uint32_t initvalue, const uint8_t* pdata, uint32_t datalen)
{

    /*- 变量初始化 */
    uint32_t crc = initvalue;/*初始值*/
    uint32_t i = 0u;

    /* Modify Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    /*- 判断指针是否为空或者数据长度是否为0 */
    SCP_ASSERT(pdata != SCP_NULL);
    SCP_ASSERT(datalen != 0u);

    /*- 循环计算CRC值 */
    while (i < datalen)
    {
        crc = g_ulSLibVoteCrc32Table[(crc ^ *pdata) & 0xFFu] ^ (crc >> 8u);
        pdata++;
        i++;
    }
    /* Modify End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
        /* begin modified by deng 2023.11.2*/
    crc = crc ^ 0xFFFFFFFFU;
        /* end modified by deng 2023.11.2*/
    return crc;
}

/**
* @brief 计算表决crc32数值(多项式0x04c11db7，初始值为0xffffffff，结果异或系数为0x00000000，输入反转，输出反转)
* - [iSCP-SLib-SwCD-F-1804]
*
* @param[in] pdata     数据指针
* @param[in] datalen   数据长度
*
* @retval crc crc计算结果
* @see [iSCP-MPU-SwAD-F-0301][iSCP-ATO-SwAD-F-0013]
*/
uint32_t S_Crc32I(const uint8_t* pdata, uint32_t datalen)
{
    /*- 变量初始化 */
    uint32_t crc;/*crc值*/
    uint32_t initvalue = 0xffffffffu;

    /*- 循环计算CRC值 */
    crc = S_ContinueVoteCrc32(initvalue, pdata, datalen);

    return crc;
}

/* begin modified by deng 2023.10.27 */
//uint32_t S_ContinueCrc32C(const uint8_t* pData, uint32_t DataLen)
uint32_t S_ContinueCrc32C(uint32_t initvalue, const uint8_t* pData, uint32_t DataLen)
{
    /*- 变量初始化 */
//    uint32_t Crc = 0xffffffffu;
      uint32_t Crc = initvalue;
/* end modified by deng 2023.10.27 */
    uint32_t i = 0u;


    /* Modify Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    /*- 判断指针是否为空或者数据长度是否为0 */
    SCP_ASSERT(pData != SCP_NULL);
    SCP_ASSERT(DataLen != 0u);

    /*- 循环计算CRC值 */
    while (i < DataLen)
    {
        Crc = g_ulSLibCrc32CTable[(Crc ^ *pData) & 0xFFu] ^ (Crc >> 8u);
        pData++;
        i++;
    }
    /* Modify End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */

    return ~Crc;
}

uint32_t S_Crc32C(const uint8_t *pdata, uint32_t datalen)
{
	/*- 变量初始化 */
	uint32_t crc;/*crc值*/
	uint32_t initvalue = 0xffffffffu;

	/*- 循环计算CRC值 */
	crc = S_ContinueCrc32C(initvalue, pdata, datalen);

	return crc;
}

/**
* @brief 将数组中的数据按大小端转换为uint16_t数据
* - [iSCP-SLib-SwCD-F-1885]
*  @param[in]  aucData 数组指针
*  @param[in]  bIsBigEndian 数组中的数据是否为大端排列
*
*  @retval  转换后的uint16_t数据
*
*  @see [iSCP-MPU-SwAD-F-0309] [iSCP-ATO-SwAD-F-0046]
*/
uint16_t S_CharToUint16(const uint8_t aucData[], bool_t bIsBigEndian)
{
    uint16_t usRet;
    memcpy(&usRet, aucData, sizeof(usRet)); /* parasoft-suppress  MISRA2004-20_3 "调用库函数前受入参限制，无需检查" */
    if (SCP_TRUE == bIsBigEndian)
    {
        usRet = ntohs(usRet);
    }
    return usRet;
}

/**
* @brief 将数组中的数据按大小端转换为uint32_t数据
* - [iSCP-SLib-SwCD-F-1886]
*  @param[in]  aucData 数组指针：not Null
*  @param[in]  bIsBigEndian 数组中的数据是否为大端排列
*
*  @retval  转换后的uint32_t数据
*
*  @see [iSCP-MPU-SwAD-F-0309] [iSCP-ATO-SwAD-F-0046]
*/
uint32_t S_CharToUint32(const uint8_t aucData[], bool_t bIsBigEndian)
{
    uint32_t ulRet;
    memcpy(&ulRet, aucData, sizeof(ulRet)); /* parasoft-suppress  MISRA2004-20_3 "调用库函数前受入参限制，无需检查" */
    if (SCP_TRUE == bIsBigEndian)
    {
        ulRet = ntohl(ulRet);
    }
    return ulRet;
}

/**
* @brief 将uint16_t数据按大小端转换为数组
* - [iSCP-SLib-SwCD-F-1887]
*  @param[in]  usData 数据
*  @param[in]  bIsBigEndian 转化的数组是否按照大端排列
*  @param[out] aucData 数组指针
*
*  @retval  转换后的uint16_t数据
*  @see [iSCP-MPU-SwAD-F-0309] [iSCP-ATO-SwAD-F-0046]
*/
void S_Uint16ToChar(uint16_t usData, bool_t bIsBigEndian, uint8_t aucData[])
{
    if (SCP_TRUE == bIsBigEndian)
    {
        usData = htons(usData);
    }
    memcpy(aucData, &usData, sizeof(usData));
    return;
}

/**
* @brief 将uint32_t数据按大小端转换为数组
* - [iSCP-SLib-SwCD-F-1888]
*  @param[in]  ulData 数据
*  @param[in]  bIsBigEndian 转化的数组是否按照大端排列
*  @param[out] aucData 数组指针
*
*  @retval  转换后的uint32_t数据
*
*  @see [iSCP-MPU-SwAD-F-0309] [iSCP-ATO-SwAD-F-0046]
*/
void S_Uint32ToChar(uint32_t ulData, bool_t bIsBigEndian, uint8_t aucData[])
{
    if (SCP_TRUE == bIsBigEndian)
    {
        ulData = htonl(ulData);
    }
    memcpy(aucData, &ulData, sizeof(ulData));
    return;
}

/* begin modified by deng 2023.10.27 */
/**
 * @brief 内存赋值
 * - [iSCP-SLib-SwCD-F-1836]
 * - 复制字符 c（一个无符号字符）到参数 str 所指向的字符串的前 n 个字符
 *
 * @param[in] str 源地址
 * @param[in] c   字符串
 * @param[in] n   字符串的前n个
 *
 * @return str 返回前n个字符串
 * @see [iSCP-MPU-SwAD-F-0302] [iSCP-ATO-SwAD-F-0014]
 */
void *S_Memset(void *str, uint8_t c, uint32_t n)
{
#ifdef __LIBC_ENABLE__
    return memset(str, (int32_t)c, n);
#else
    uint32_t   size_rem;
    uint32_t   data_align;
    uint32_t   *pmem_align;
    uint8_t    *pmem_08;
    uint32_t   mem_align_mod;
    uint32_t   i;

    /* Add Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    SCP_ASSERT(str != SCP_NULL);
    /* Add End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */

    if ((n < 1) || (str == (void *)0)) {
        return str;
    }

    data_align = 0u;
    for (i = 0u; i < sizeof(uint32_t); i++) {
        data_align <<=  8u;
        data_align  |= (uint32_t)c;
    }

    size_rem      =  n;
    mem_align_mod = (uint8_t)((uint32_t)str % sizeof(uint32_t));

    pmem_08 = (uint8_t *)str;
    if (mem_align_mod != 0u) {
        i = mem_align_mod;
        while ((size_rem > 0) &&
               (i        < sizeof(uint32_t ))) {
           *pmem_08++ = c;
            size_rem -= sizeof(uint8_t);
            i++;
        }
    }

    pmem_align = (uint32_t *)pmem_08;
    while (size_rem >= sizeof(uint32_t)) {
       *pmem_align++ = data_align;
        size_rem    -= sizeof(uint32_t);
    }

    pmem_08 = (uint8_t *)pmem_align;
    while (size_rem > 0) {
       *pmem_08++   = c;
        size_rem   -= sizeof(uint8_t);
    }
    return str;
#endif
}
/* Modify End   by zhuchengyin 2023年5月22日 pn:不使用库函数 */

/**
 * @brief 内存数据清零函数
 *  - 复制字符 0 到参数 pStr 所指向的字符串的前 ulLen 个字符
 *  -[iSCP-SLib-SwCD-F-1877]
 * @param[in]   str 内存数据地址
 * @param[in]   n 设置长度
 * @return 返回内存数据地址
 * @see [iSCP-MPU-SwAD-F-0302] [iSCP-ATO-SwAD-F-0014]
 */
void *S_MemZero(void *str, uint32_t n)
{
    /* Add Begin by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */
    SCP_ASSERT(str != SCP_NULL);
    /* Add End   by zhuchengyin 2023年9月4日 pn:正式发布第一个版本 */

    return S_Memset(str, 0u, n);
}

/**
 * @brief 获取平台本地时间
 * - [iSCP-SLib-SwCD-F-1867]
 * @see [iSCP-MPU-SwAD-F-0306][iSCP-ATO-SwAD-F-0018]
 *
 * @return 返回平台本地时间
 */
uint32_t S_GetLocalTime(void)
{
    return OSTimeGet();
}
/* end modified by deng 2023.10.27 */
/* begin modified by deng 2023.11.6 */
/**
 * @brief 异或校验
 *
 * @return 返回异或值
 */
uint8_t S_CheckXOR(uint8_t* pucDataBuf, int32_t lDataLen) 
{  
    int32_t lNum;  
    uint8_t ucXOR = 0;  
    for (lNum = 0; lNum < lDataLen; lNum++) 
    {  
        ucXOR ^= pucDataBuf[lNum];  
    }  
    return ucXOR;  
}

/**
 * @brief 取绝对值
 */
uint32_t S_ABS(int32_t lData) 
{   
    return abs(lData);  
}
/* end modified by deng 2023.11.6  */
