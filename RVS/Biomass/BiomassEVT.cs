using System;
using System.Collections.Generic;
using System.Data;

namespace RVS.Biomass
{
    /// <summary>
    /// BiomassEVT inherits from EVT and contains attributes needed to
    /// calculate biomass. 
    /// </summary>
    public class BiomassEVT : DataManagement.EVT
    {
        protected string dom_spp;
        protected string spp_code;
        protected BiomassReturnType returnType;
        protected float percent_dom;
        protected BiomassVarUnits pa1_code;
        protected float pa1_val;
        protected BiomassVarUnits pa2_code;
        protected float pa2_val;
        protected BiomassVarUnits pa3_code;
        protected float pa3_val;

        protected Dictionary<BiomassVarUnits, float> parms;
        //protected Dictionary<string, float> results;

        public string DOM_SPP { get { return dom_spp; } }
        public string SPP_CODE { get { return spp_code; } }
        public BiomassReturnType RETURNTYPE { get { return returnType; } }
        public float PERCENT_DOM { get { return percent_dom; } }
        public Dictionary<BiomassVarUnits, float> PARAMETERS { get { return parms; } }
        //public Dictionary<string, float> RESULTS { get { return results; } }

        /// <summary>
        /// Blank constructor.
        /// </summary>
        public BiomassEVT() : base()
        {
            this.initialize();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="dt"></param>
        /// <param name="row_num"></param>
        public BiomassEVT(ref DataTable dt, int row_num) : base()
        {
            this.initialize();
            this.buildEVT(ref dt, row_num);
        }

        /// <summary>
        /// Variable initializer. Called by all Constructors.
        /// </summary>
        new protected void initialize()
        {
            dom_spp = "";
            spp_code = "";
            returnType = BiomassReturnType.NUL;
            percent_dom = 0f;
            pa1_code = BiomassVarUnits.NUL;
            pa1_val = 0f;
            pa2_code = BiomassVarUnits.NUL;
            pa2_val = 0f;
            pa3_code = BiomassVarUnits.NUL;
            pa3_val = 0f;

            parms = new Dictionary<BiomassVarUnits, float>();
            //results = new Dictionary<string, float>();
        }

        /// <summary>
        /// A better buildEVT function that doesn't depend on column positions
        /// </summary>
        /// <param name="dt"></param>
        /// <param name="row_num"></param>
        protected override void buildEVT(ref DataTable dt, int row_num)
        {
            this.parseItem(dt.Rows[row_num]["evt_num"], ref evt_num);
            this.parseItem(dt.Rows[row_num]["bps"], ref bps_num);
            this.parseItem(dt.Rows[row_num]["lifeform"], ref lifeform);
            this.parseItem(dt.Rows[row_num]["spp_code"], ref spp_code);
            this.parseItem(dt.Rows[row_num]["dom_spp"], ref dom_spp);
            this.parseItem(dt.Rows[row_num]["plots"], ref samplePlots);
            this.parseItem(dt.Rows[row_num]["tplots"], ref totalSamplePlots);

            this.parseItem(dt.Rows[row_num]["pa1_val"], ref pa1_val);
            this.parseItem(dt.Rows[row_num]["pa2_val"], ref pa2_val);
            this.parseItem(dt.Rows[row_num]["pa3_val"], ref pa3_val);

            this.parseItem(dt.Rows[row_num]["pa1_code"], ref pa1_code);
            this.parseItem(dt.Rows[row_num]["pa2_code"], ref pa2_code);
            this.parseItem(dt.Rows[row_num]["pa3_code"], ref pa3_code);

            this.parseItem(dt.Rows[row_num]["ret_code"], ref returnType);

            if (pa1_code != BiomassVarUnits.NUL)
                parms.Add(pa1_code, pa1_val);
            if (pa2_code != BiomassVarUnits.NUL)
                parms.Add(pa2_code, pa2_val);
            if (pa3_code != BiomassVarUnits.NUL)
                parms.Add(pa3_code, pa3_val);
        }

        protected void parseItem(object inObj, ref BiomassVarUnits unit)
        {
            try 
            {
                if (!(inObj is System.DBNull))
                {
                    string inStr = Convert.ToString(inObj);
                    inStr = inStr.ToUpper();
                    unit = (BiomassVarUnits)Enum.Parse(typeof(BiomassVarUnits), inStr);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(ex.Message, "BiomassVarUnits Parse");
            }
        }
        protected void parseItem(object inObj, ref BiomassReturnType ret)
        {
            try
            {
                if (!(inObj is System.DBNull))
                {
                    string inStr = Convert.ToString(inObj);
                    inStr = inStr.ToUpper();
                    ret = (BiomassReturnType)Enum.Parse(typeof(BiomassReturnType), inStr);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(ex.Message, "BiomassReturnType Parse");
            }
        }
    }
}
