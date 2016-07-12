using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VMDNS.Client;

namespace VMDNSSnapIn.UI.RecordViews
{
    class RecordFactory
    {
        Dictionary<RecordType, RecordClassBase> RecordObjects = new Dictionary<RecordType, RecordClassBase>() { 
        {RecordType.VMDNS_RR_TYPE_A, new RecordClassA()},
            {RecordType.VMDNS_RR_TYPE_AAAA, new RecordClassAAAA()},
            {RecordType.VMDNS_RR_TYPE_CNAME,new RecordClassCName()},
            {RecordType.VMDNS_RR_TYPE_NS, new RecordClassNS()},
            {RecordType.VMDNS_RR_TYPE_PTR, new RecordClassPtr()},
            {RecordType.VMDNS_RR_TYPE_SRV, new RecordClassSRV()},
            {RecordType.VMDNS_RR_TYPE_SOA, new RecordClassSOA()}
         };

        public RecordClassBase GetRecord(RecordType recordType)
        {
            return RecordObjects[recordType];
        }
    }
}
