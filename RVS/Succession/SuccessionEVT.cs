using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;

namespace RVS.Succession
{
    public class SuccessionEVT : DataManagement.EVT
    {
        public SuccessionEVT() : base()
        {
            this.initialize();
        }
        public SuccessionEVT(ref DataTable dt, int row_num)
        {
            this.initialize();
            this.buildEVT(ref dt, row_num);
        }

        new protected void initialize()
        {
        }

        protected override void buildEVT(ref DataTable dt, int row_num)
        {
            throw new NotImplementedException();
        }
    }
}
