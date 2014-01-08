
namespace RVS.DataManagement
{
    public static class TypeDefs
    {
        /// <summary>
        /// TODO: Fill this in.
        /// </summary>
        /// <param name="input"></param>
        /// <returns></returns>
        public static Lifeform parseLifeform(string input)
        {
            return Lifeform.unk;

        }
    }

    /// <summary>
    /// Enumeration of lifeform types. Used for choosing Biomass calculation method
    /// and succession/growth equations.
    /// </summary>
    public enum Lifeform { 
        forb, 
        graminoid,
        herb, 
        liana,
        lichenous, 
        nonvascular, 
        shrub,
        subshrub, 
        tree, 
        unk, 
        vine }
}
