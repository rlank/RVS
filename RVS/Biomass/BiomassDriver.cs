using System;

namespace RVS.Biomass
{
    /// <summary>
    /// BiomassDriver is the primary tool for calculating biomass. It expects an
    /// array of BiomassEVT for a single analysis plot. If the plot contains 
    /// only 1 EVT/DomSpp record, then it's just an array of 1.  
    /// </summary>
    public class BiomassDriver
    {
        private BiomassEVT[] evt_records;
        private BiomassLookupLevel level;
        public bool suppress_messages;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="level">The lookup level to use for primary production (herb biomass)</param>
        /// <param name="suppress_messages">Optional. Toggle to 'true' to suppress console messages. Useful when threading.</param>
        public BiomassDriver(BiomassLookupLevel level, bool suppress_messages = false)
        {
            this.level = level;
            this.suppress_messages = suppress_messages;
        }

        /// <summary>
        /// Main function. Pass a return value and type reference, and BioMain sets them upon completion.
        /// No other function needs to be called to calculate biomass.
        /// </summary>
        /// <param name="plot_num">Analysis plot ID</param>
        /// <param name="biomass_return_value">The calculated biomass of the plot.</param>
        /// <param name="biomass_return_type">The units of the calculated biomass.</param>
        /// <returns>Return code. 0 indicates a clean run.</returns>
        public int BioMain(int plot_num, ref double biomass_return_value, ref BiomassReturnType biomass_return_type)
        {
            int RETURN_STATE = 0;
            try
            {
                if (!suppress_messages)
                {
                    Console.WriteLine();
                    Console.WriteLine("Results for plot " + plot_num.ToString());
                    Console.WriteLine("====================");
                }

                // declare local return vars (these will be printed to console)
                double BIOMASS_RETURN_VAL = 0d;
                Biomass.BiomassReturnType BIOMASS_RET_TYPE = Biomass.BiomassReturnType.NUL;

                double primary_production = 0d;
                double herbBiomass = 0d;
                double shrubBiomass = 0d;

                // read in data. the query_biomass_input_table function returns a DataTable
                // with only records for that plot number
                System.Data.DataTable dt = DataManagement.DIO.query_biomass_input_table(plot_num);

                // create BiomassEVT array for input into BiomassDriver
                evt_records = new Biomass.BiomassEVT[dt.Rows.Count];

                for (int row_counter = 0; row_counter < dt.Rows.Count; row_counter++)
                {
                    evt_records[row_counter] = new Biomass.BiomassEVT(ref dt, row_counter);

                    if (row_counter == 0)
                    {
                        primary_production = this.calcHerbBiomass(ref evt_records[row_counter]);
                    }

                    double singleBiomass = 0d;

                    // Branch biomass calculation to do either HERB or SHRUB
                    switch (evt_records[row_counter].LIFEFORM)
                    {
                        case DataManagement.Lifeform.shrub:
                            singleBiomass = this.calcShrubBiomass(ref evt_records[row_counter]);
                            //evt_records[row_counter].RESULTS.Add("biomass", (float)singleBiomass);
                            break;
                        case DataManagement.Lifeform.herb:
                            //singleBiomass = this.calcHerbBiomass(ref evt_records[row_counter]);
                            break;
                    }

                    shrubBiomass += singleBiomass;
                }

                herbBiomass = primary_production - shrubBiomass;
                BIOMASS_RETURN_VAL = shrubBiomass + herbBiomass;
                BIOMASS_RET_TYPE = BiomassReturnType.PCH;

                // report biomass for plot. for now this means dump to console.

                if (!suppress_messages)
                {
                    Console.WriteLine();
                    Console.WriteLine("Primary Production: " + primary_production.ToString());
                    Console.WriteLine("Shrub Biomass: " + shrubBiomass.ToString());
                    Console.WriteLine("Herb Biomass: " + herbBiomass.ToString());
                    Console.WriteLine("Total Biomass: " + BIOMASS_RETURN_VAL.ToString());
                    Console.WriteLine("Units: " + BIOMASS_RET_TYPE);
                    Console.WriteLine("Return State: " + RETURN_STATE.ToString());
                }

                // cleanup
                evt_records = null;

            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(ex.Message);
                RETURN_STATE = 1;
            }

            return RETURN_STATE;
        }

        private bool checkShrubInput()
        {
            bool useLookuptableInstead = false;

            return useLookuptableInstead;
        }

        private double calcShrubBiomass(ref BiomassEVT evt)
        {
            double biomass = 0d;

            if (this.checkShrubInput() == true)
            {
                biomass = this.calcHerbBiomass(ref evt);
                return biomass;
            }

            string equationCode = evt.RETURNTYPE.ToString();

            System.Data.DataTable dt;
            dt = DataManagement.DIO.query_biomass_crosswalk_table(evt.SPP_CODE);

            int equationIndex = 0;
            DataManagement.DIO.parseItem(dt.Rows[0][equationCode], ref equationIndex);

            //if (!suppress_messages)
            //    Console.WriteLine("equ_code: " + equationIndex.ToString());

            dt = DataManagement.DIO.query_biomass_equation_table(equationIndex);

            string equationstring = "";
            DataManagement.DIO.parseItem(dt.Rows[0]["EQ_FRM"], ref equationstring);

            double cf1 = 0d;
            double cf2 = 0d;
            double cf3 = 0d;
            double cf4 = 0d;

            DataManagement.DIO.parseItem(dt.Rows[0]["CF1"], ref cf1);
            DataManagement.DIO.parseItem(dt.Rows[0]["CF2"], ref cf2);
            DataManagement.DIO.parseItem(dt.Rows[0]["CF3"], ref cf3);
            DataManagement.DIO.parseItem(dt.Rows[0]["CF4"], ref cf4);

            double height = evt.PARAMETERS[BiomassVarUnits.HT];

            double pch = BiomassEquations.eq_PCH(cf1, cf2, height);
            double pch_acre = (pch / 1000) * 4046.85642;

            if (!suppress_messages)
            {
                Console.WriteLine(evt.DOM_SPP + " CF1: " + cf1.ToString());
                Console.WriteLine(evt.DOM_SPP + " CF2: " + cf2.ToString());
                Console.WriteLine(evt.DOM_SPP + " HT: " + evt.PARAMETERS[BiomassVarUnits.HT].ToString());
                Console.WriteLine(evt.DOM_SPP + " COV: " + evt.PARAMETERS[BiomassVarUnits.COV].ToString());
                Console.WriteLine(evt.DOM_SPP + " PCH: " + pch.ToString());
            }
            

            biomass = pch_acre * (evt.PARAMETERS[BiomassVarUnits.COV] / 100);

            //Biomass.BiomassEquations.read(equationstring);
            //string temp = Biomass.BiomassEquations.getToken(equationstring);


            return biomass;
        }

        /// <summary>
        /// This is the primary biomass calculator for herbs and the fallback for shrubs
        /// (used when shrub equation parsing fails). Loads the Bio_Herbs table and 
        /// looks up the respective biomass value.
        /// </summary>
        /// <param name="bps">5-digit BPS number (without map zone)</param>
        /// <returns>Biomass</returns>
        private double calcHerbBiomass(ref BiomassEVT evt)
        {
            string bps = evt.BPS_NUM.Substring(2);
            int baseBPS = int.Parse(bps);

            System.Data.DataTable dt = DataManagement.DIO.query_biomass_herbs_table(baseBPS);
            double biomass = 0d;
            switch (this.level)
            {
                case BiomassLookupLevel.low:
                    biomass = Convert.ToDouble(dt.Rows[0]["low_val"]);
                    break;
                case BiomassLookupLevel.medium:
                    biomass = Convert.ToDouble(dt.Rows[0]["med_val"]);
                    break;
                case BiomassLookupLevel.high:
                    biomass = Convert.ToDouble(dt.Rows[0]["high_val"]);
                    break;
            }
            return biomass;
        }
    }
}
