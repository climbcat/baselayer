#include "../resource.h"


//  In this example, we have three font atlas instances inlined in the hexed data below:
//  The default name is meant as the recommended starting point / initialization value.


static HexResourceHdr cmunrm[6] = {
    { RT_RESOURCE_CNT, 1, "" },
    { RT_RESOURCE_NAME, 0, "cmunrm" },
    { RT_FONT, 180280, "cmunrm_48" },
    { RT_RESOURCE_DATA, 180280*2, "600200002001000018D4598..." }
};
