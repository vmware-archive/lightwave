using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace VMAFD.Client
{
    public enum CDC_ADDRESS_TYPE
    {
        CDC_ADDRESS_TYPE_UNDEFINED = 0,
        CDC_ADDRESS_TYPE_INET,
        CDC_ADDRESS_TYPE_NETBIOS
    };

    public enum CDC_DC_STATE
    {
        CDC_DC_STATE_UNDEFINED =0,
        CDC_DC_STATE_NO_DC_LIST,
        CDC_DC_STATE_SITE_AFFINITIZED,
        CDC_DC_STATE_OFF_SITE,
        CDC_DC_STATE_NO_DCS_ALIVE,
        CDC_DC_STATE_LEGACY
    };
}
