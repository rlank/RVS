using System;
using System.Data;

namespace RVS.DataManagement
{
    /// <summary>
    /// 
    /// </summary>
    public static class DIO
    {
        public static string pathToDb = "";
        private static string accessConnectionString = "Provider=Microsoft.ACE.OLEDB.12.0;data source=";

        /// <summary>
        /// Base query function. All the public functions only define the selection string.
        /// This function contains the actual OleDB stuff.
        /// </summary>
        /// <param name="selectString"></param>
        /// <returns></returns>
        private static DataTable query_base(string selectString)
        {
            System.Data.OleDb.OleDbConnection conn = null;
            string connectionString = accessConnectionString + pathToDb;
            DataTable results = new DataTable();

            try
            {
                conn = new System.Data.OleDb.OleDbConnection(connectionString);
                System.Data.OleDb.OleDbCommand cmd = new System.Data.OleDb.OleDbCommand(selectString, conn);
                conn.Open();
                System.Data.OleDb.OleDbDataAdapter adapter = new System.Data.OleDb.OleDbDataAdapter(cmd);
                adapter.Fill(results);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(ex.Message);
            }
            finally
            {
                conn.Close();
            }
            return results;
        }

        /// <summary>
        /// Alternative base query method. Contains "using" block instead of try/finally. Not as cool.
        /// </summary>
        /// <param name="selectString"></param>
        /// <returns></returns>
        private static DataTable query_base_alt(string selectString)
        {
            string connectionString = accessConnectionString + pathToDb;
            DataTable results = new DataTable();
            using (System.Data.OleDb.OleDbConnection conn = new System.Data.OleDb.OleDbConnection(connectionString))
            {
                System.Data.OleDb.OleDbCommand cmd = new System.Data.OleDb.OleDbCommand(selectString, conn);
                conn.Open();
                System.Data.OleDb.OleDbDataAdapter adapter = new System.Data.OleDb.OleDbDataAdapter(cmd);
                adapter.Fill(results);
            }
            return results;
        }

        /// <summary>
        /// Queries Biomass_Input table from RVSDB for records matching a passed
        /// plot number. Biomass_Input should contain information about dominant
        /// species, codes for input variables, and values of input variables.
        /// </summary>
        /// <param name="plot_num">Plot number to query for.</param>
        /// <returns>DataTable with matching plot records.</returns>
        public static DataTable query_biomass_input_table(int plot_num)
        {
            string selectString = "SELECT * FROM Biomass_Input WHERE plot_num=" + plot_num.ToString();
            return DIO.query_base(selectString);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="spp_code"></param>
        /// <returns></returns>
        public static DataTable query_biomass_equation_table(string spp_code)
        {
            string selectString = "SELECT * FROM Bio_Equation_Import WHERE SPP_CODE='" + spp_code + "'";
            return DIO.query_base(selectString);
        }

        public static DataTable query_biomass_equation_table(int equation_number)
        {
            string selectString = "SELECT * FROM Bio_Equation_Import WHERE EQN_NO=" + equation_number.ToString();
            return DIO.query_base(selectString);
        }

        public static DataTable query_biomass_crosswalk_table(string spp)
        {
            string selectString = "SELECT * FROM Bio_Crosswalk WHERE spp_code='" + spp + "'";
            return DIO.query_base(selectString);
        }

        /// <summary>
        /// Queries Bio_Herbs table to lookup herbaceous biomass. Takes a BPS number to query against.
        /// </summary>
        /// <param name="baseBPS">5-digit BPS number</param>
        /// <returns>A single row DataTable with biomass record.</returns>
        public static DataTable query_biomass_herbs_table(int baseBPS)
        {
            string selectString = "SELECT * FROM Bio_Herbs WHERE BPS_CODE=" + baseBPS.ToString();
            return DIO.query_base(selectString);
        }

        /// <summary>
        /// Returns an array of unique analysis plot values. Use this array to control main driver
        /// iteration. Each analysis plot can contain multiple records for different EVT/SPP combos
        /// </summary>
        /// <returns>Array of analysis plot values</returns>
        public static int[] query_analysis_plots()
        {
            string selectString = "SELECT DISTINCT plot_num FROM RVS_Input_Use";
            DataTable dt = DIO.query_base(selectString);
            int[] retAry = new int[dt.Rows.Count];
            for (int i = 0; i < retAry.Length; i++)
            {
                retAry[i] = (int)(short)dt.Rows[i]["plot_num"];
            }
            return retAry;
        }


        public static void parseItem(object inInt, ref int outInt)
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
        public static void parseItem(object inStr, ref string outStr)
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
        public static void parseItem(object inFloat, ref float outFloat)
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
        public static void parseItem(object inDouble, ref double outDouble)
        {
            try
            {
                if (!(inDouble is System.DBNull))
                {
                    if (inDouble is double) { outDouble = (double)inDouble; }
                    else { outDouble = (double)(float)inDouble; }
                    //outFloat = Convert.ToSingle(inFloat);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(ex.Message, "Double Parse");
            }
        }
    }
}
