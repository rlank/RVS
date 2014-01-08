
namespace RVS.Biomass
{
    /// <summary>
    /// Enumeration of return units for shrub allometries
    /// </summary>
    public enum BiomassReturnType
    {
        AFN, // New all-sided leaf area (cm^2)
        AFT, // Total all-sided leaf area (cm^2)
        BAE, // Total aboveground biomass (g) (more specific definition than BAT)
        BAL, // Aboveground live biomass (g)
        BAP, // Aboveground live biomass, wood + bark (g)
        BAT, // Total aboveground biomass (g)
        BBD, // Dead branch biomass (g)
        BBL, // Live branch biomass (g)
        BFN, // New foliage biomass (g)
        BFT, // Total foliage biomass (g)
        BRL, // Total root biomass (g)
        BST, // Total stem biomass (g)
        BUN, // Nodule biomass, underground (g)
        PCH, // Projected area, crown, horizontal surface (cm^2)
        PFN, // New projected leaf area (cm^2)
        PFT, // Total projected leaf area (cm^2)
        VST, // Bole volume, bark and wood (cm^3)
        NUL  // None/void type
    }

    /// <summary>
    /// Enumeration of parameter input types for shrub allometries
    /// </summary>
    public enum BiomassVarUnits
    {
        BIO, // Total aboveground biomass
        COV, // Cover (% in decimal i.e. .4 for 40%)
        DBA, // Stem basal diameter (cm)
        HT,  // Height (cm)
        LEN, // Length or Canopy Length (cm)
        NUM, // Number of basal stems (count)
        SPA, // Sapwood cross-section area at ground level (cm)
        VOL, // Volume (dm^3) (if not given directly, VOL = LEN * WID * HT
        WID, // Width (cm)
        NUL  // None/Void type
    }

    /// <summary>
    /// Enumeration of herb lookup level
    /// </summary>
    public enum BiomassLookupLevel { high, medium, low }
}
