using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace RVS.Biomass
{
    public static class BiomassEquations
    {

        public static void read(string activestring)
        {
            
            // get return (first) part
            string[] sub;
            sub = activestring.Split('+','=','*');

            //int currPos = 0;
            char[] chars = activestring.ToCharArray();
            
        }

        public static void subNumbers(ref string inStr, Dictionary<BiomassVarUnits, float> parms, 
            double coef1 = 0d, 
            double coef2 = 0d, 
            double coef3 = 0d, 
            double coef4 = 0d )
        {
            if (inStr.IndexOf("CF1") > 0)
                inStr.Replace("CF1", coef1.ToString());
            if (inStr.IndexOf("CF2") > 0)
                inStr.Replace("CF2", coef2.ToString());
            if (inStr.IndexOf("CF3") > 0)
                inStr.Replace("CF3", coef3.ToString());
            if (inStr.IndexOf("CF4") > 0)
                inStr.Replace("CF4", coef4.ToString());


        }

        public static string getToken(string inStr)
        {
            string retStr = "";
            
            char[] chars = inStr.ToCharArray();
            bool isNumber = false;

            if (Char.IsDigit(chars[0]))
            {
                isNumber = true;
            }

            for (int i = 0; i < inStr.Length; i++)
            {
                if (Char.IsDigit(chars[i]) == isNumber)
                {
                    retStr += chars[i].ToString();
                }
                else
                {
                    break;
                }
            }

            return retStr;
        }


        //// ## Hard Coded Versions ## \\\\
        public static double eqn_76()
        {
            return 0d;
        }
        public static double eqn_78()
        {
            return 0d;
        }
        public static double eqn_91()
        {
            return 0d;
        }
        public static double eqn_98()
        {
            return 0d;
        }
        public static double eqn_199()
        {
            return 0d;
        }
        public static double eqn_201()
        {
            return 0d;
        }
        public static double eqn_246()
        {
            return 0;
        }

        public static double eq_PCH(double cf1, double cf2, double height)
        {
            double result = 0d;
            result = cf1 + cf2 * Math.Log10(height);
            result = Math.Pow(10, result);
            return result;
        }

    }
}
