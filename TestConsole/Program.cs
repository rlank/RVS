using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace TestConsole
{
    class Program
    {
        static int RETURN_STATE = 0;
        static bool USE_THREADING = false;
        static bool SUPPRESS_MSG = false;

        static void Main(string[] args)
        {
            // TODO: ARGS HANDLER

            string consoleRead = "";


            // ## UNCOMMENT following lines to distribute test version ## //
            Console.Write("Enter path to RVS database or leave blank to use default: (C:\\Reeves\\rvs_data.accdb) ");
            RVS.DataManagement.DIO.pathToDb = Console.ReadLine();
            Console.Write("Run Mode Options: "
                + "\nVN: Verbose, no threading (default)"
                + "\nVT: Verbose, with threading"
                + "\nSN: Silent, no threading"
                + "\nST: Silent, with threading"
                + "\n: ");
            consoleRead = Console.ReadLine();

            switch (consoleRead)
            {
                case "VT":
                    SUPPRESS_MSG = false;
                    USE_THREADING = true;
                    break;
                case "VN":
                    SUPPRESS_MSG = false;
                    USE_THREADING = false;
                    break;
                case "ST":
                    SUPPRESS_MSG = true;
                    USE_THREADING = true;
                    break;
                case "SN":
                    SUPPRESS_MSG = true;
                    USE_THREADING = false;
                    break;
                default:
                    SUPPRESS_MSG = false;
                    USE_THREADING = false;
                    break;
            }

            Console.WriteLine();


            DateTime startTime = DateTime.Now;

            // setup static DIO class
            if (RVS.DataManagement.DIO.pathToDb.Length <= 2)
            {
                //DataManagement.DIO.pathToDb = "C:\\MCR\\RVS\\Data\\rvs_data.accdb";
                RVS.DataManagement.DIO.pathToDb = "C:\\Reeves\\rvs_data.accdb";
            }

            ////////////////////////////////////////////////////////////////////////////////
            // This is a Biomass test, so build an array of BiomassEVT and run this thing //
            ////////////////////////////////////////////////////////////////////////////////

            int[] plotcounts = RVS.DataManagement.DIO.query_analysis_plots();

            List<Thread> tList = new List<Thread>();
            Dictionary<int, double> biomassResults = new Dictionary<int, double>();

            for (int plot_num = 0; plot_num < plotcounts.Length; plot_num++)
            {
                double biomass = 0d;
                int runplot = plotcounts[plot_num];
                RVS.Biomass.BiomassReturnType retType = RVS.Biomass.BiomassReturnType.NUL;
                RVS.Biomass.BiomassDriver bdriver = new RVS.Biomass.BiomassDriver(RVS.Biomass.BiomassLookupLevel.medium, SUPPRESS_MSG);

                if (USE_THREADING)
                {
                    Thread tt = new Thread(delegate()
                        {
                            RETURN_STATE = bdriver.BioMain(runplot, ref biomass, ref retType);
                        });
                    tt.Name = "RVS_P" + plot_num.ToString();
                    tt.Start();
                    tList.Add(tt);
                }
                else
                {
                    RETURN_STATE = bdriver.BioMain(runplot, ref biomass, ref retType);
                }

                //biomassResults.Add(runplot, biomass);
            }

            foreach (Thread t in tList)
            {
                t.Join();
            }

            //if (USE_THREADING)
            //{
            //    foreach (KeyValuePair<int, double> item in biomassResults)
            //    {
            //        Console.WriteLine("Plot " + item.Key.ToString() + " : " + item.Value.ToString());
            //    }
            //}

            DateTime stopTime = DateTime.Now;
            TimeSpan ttime = stopTime - startTime;

            Console.WriteLine("\nTest run complete.");
            Console.WriteLine("Time: " + ttime.ToString());
            Console.ReadLine();
        }
    }
}
