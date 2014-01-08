using System;

namespace RVS.DataManagement
{
    /// <summary>
    /// EVT base class.
    /// </summary>
    public abstract class EVT
    {
        protected int evt_num;
        protected string bps_num;
        protected Lifeform lifeform;
        protected int samplePlots;
        protected int totalSamplePlots;

        public int EVT_NUM { get { return evt_num; } }
        public string BPS_NUM { get {return bps_num; } }
        public Lifeform LIFEFORM { get { return lifeform; } }
        public int SAMPLEPLOTS { get { return samplePlots; } }
        public int TOTALSAMPLEPLOTS { get { return totalSamplePlots; } }

        public EVT()
        {
            this.initialize();
        }
        //public EVT(ref System.Data.DataTable dt, int row_num)
        //{
        //    this.initialize();
        //}

        /// <summary>
        /// Variable initializer. Called by all constructors.
        /// </summary>
        protected virtual void initialize()
        {
            evt_num = 0;
            bps_num = "";
            lifeform = Lifeform.unk;
            samplePlots = 0;
            totalSamplePlots = 0;
        }

        /// <summary>
        /// Abstract buildEVT. Implement this method to populate inherited EVT attributes.
        /// </summary>
        /// <param name="dt"></param>
        /// <param name="row_num"></param>
        protected abstract void buildEVT(ref System.Data.DataTable dt, int row_num);

        protected virtual void parseItem(object inInt, ref int outInt)
        {
            try 
            {
                if (!(inInt is System.DBNull))
                {
                    if (inInt is short) { outInt = (int)(short)inInt; }
                    else { outInt = (int)inInt; }
                    //outInt = Convert.ToInt32(inInt);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(ex.Message, "Int Parse");
            }
        }
        protected virtual void parseItem(object inStr, ref string outStr)
        {
            try 
            {
                if (!(inStr is System.DBNull))
                {
                    outStr = Convert.ToString(inStr); 
                }
                
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(ex.Message, "String Parse");
            }
        }
        protected virtual void parseItem(object inFloat, ref float outFloat)
        {
            try 
            {
                if (!(inFloat is System.DBNull))
                {
                    if (inFloat is double) { outFloat = (float)(double)inFloat; }
                    else { outFloat = (float)inFloat; }
                    //outFloat = Convert.ToSingle(inFloat);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(ex.Message, "Float Parse");
            }
        }
        protected virtual void parseItem(object inObj, ref Lifeform lf)
        {
            try
            {
                if (!(inObj is System.DBNull))
                {
                    string inStr = Convert.ToString(inObj);
                    inStr = inStr.ToLower();
                    lf = (Lifeform)Enum.Parse(typeof(Lifeform), inStr);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(ex.Message, "Lifeform Parse");
            }
        }

        
    }
}
