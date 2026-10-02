#include "AnalysisPlot.h"

#include <algorithm>
#include <cstdlib>
#include <utility>

using RVS::DataManagement::AnalysisPlot;

AnalysisPlot::AnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
	: plot_id(0),
	  evt_num(0),
	  bps_num(0),
	  fallback_bps_num(0),
	  latitude(0.0),
	  longitude(0.0),
	  doNotModel(false),
	  lower_confidence(0.0),
	  upper_confidence(0.0),
	  s2y(0.0),
	  shrubHeight(0.0),
	  shrubCover(0.0),
	  herbHeight(0.0),
	  herbCover(0.0),
	  totalBiomass(0.0),
	  herbBiomass(0.0),
	  herbHoldoverBiomass(0.0),
	  rawProduction(0.0),
	  primaryProduction(0.0),
	  shrubBiomass(0.0),
	  shrubAvgStem(0.0),
	  defaultFBFM(0),
	  calcFBFM(0),
	  dryClimate(true),
	  fuel1HrProp(0.0),
	  fuelFoilageProp(0.0),
	  fuel10HrProp(0.0),
	  fuel100HrProp(0.0),
	  shrub1HourWB(0.0),
	  shrub1HourFoliage(0.0),
	  shrub10Hour(0.0),
	  shrub100Hour(0.0),
	  shrub1000Hour(0.0),
	  total1HrFuel(0.0),
	  herbFuel(0.0),
	  currentStage(0),
	  plotAge(0),
	  timeInHerbStage(0),
	  disturbed(false),
	  burned(false),
	  biomassReductionTotal(0.0)
{
	previousHerbProductions.fill(0.0);

	// Order matters here!
	buildAnalysisPlot(dio, dt);
	buildInitialFuels(dio);
}

AnalysisPlot::~AnalysisPlot()
{
	syncExternalShrubPointers();
	shrubRecords.clear();
	rawShrubView.clear();
}

void AnalysisPlot::syncExternalShrubPointers()
{
	// Check if any external callers pushed new SppRecord* into rawShrubView
	for (auto* const ptr : rawShrubView)
	{
		if (!ptr)
		{
			continue;
		}

		const bool exists = std::any_of(
			shrubRecords.begin(),
			shrubRecords.end(),
			[ptr](const std::unique_ptr<SppRecord>& rec)
			{
				return rec.get() == ptr;
			});

		if (!exists)
		{
			shrubRecords.push_back(std::unique_ptr<SppRecord>(ptr));
		}
	}
}

std::vector<RVS::DataManagement::SppRecord*>* AnalysisPlot::SHRUB_RECORDS()
{
	syncExternalShrubPointers();
	rawShrubView.clear();
	rawShrubView.reserve(shrubRecords.size());
	for (const auto& rec : shrubRecords)
	{
		if (rec)
		{
			rawShrubView.push_back(rec.get());
		}
	}
	return &rawShrubView;
}

void AnalysisPlot::buildAnalysisPlot(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
{
	if (!dio || !dt)
	{
		return;
	}

	sqlite3_stmt* stmt = dt->getStmt();
	if (!stmt)
	{
		return;
	}

	dio->getVal(stmt, dt->Columns[PLOT_NUM_FIELD], &plot_id);
	dio->getVal(stmt, dt->Columns[PLOT_NAME_FIELD], &plot_name);
	dio->getVal(stmt, dt->Columns[EVT_NUM_FIELD], &evt_num);
	dio->getVal(stmt, dt->Columns[BPS_NUM_FIELD], &bps_num);
	dio->getVal(stmt, dt->Columns[BPS_MODEL_FIELD], &bps_model_num);
	dio->getVal(stmt, dt->Columns[HERB_COVER_FIELD], &herbCover);
	dio->getVal(stmt, dt->Columns[HERB_HEIGHT_FIELD], &herbHeight);
	dio->getVal(stmt, dt->Columns[SUCCESSION_CLASS_FIELD], &currentStage);

	dio->getVal(stmt, dt->Columns[LATITUDE_FIELD], &latitude);
	dio->getVal(stmt, dt->Columns[LONGITUDE_FIELD], &longitude);

	// Get NDVI and Precip values on the stack (no heap allocation!)
	for (const auto& [colName, colIdx] : dt->Columns)
	{
		if (colName.rfind("NDVI", 0) == 0) // starts_with("NDVI")
		{
			double aval = 0.0;
			dio->getVal(stmt, colIdx, &aval);
			ndviValues.push_back(aval);
		}
		else if (colName.rfind("PPT", 0) == 0) // starts_with("PPT")
		{
			double aval = 0.0;
			dio->getVal(stmt, colIdx, &aval);
			precipValues.push_back(aval);
		}
	}

	// Pull records from shrub table
	push_shrub(dio, dt);
}

void AnalysisPlot::buildInitialFuels(RVS::DataManagement::DIO* dio)
{
	if (!dio)
	{
		return;
	}
	dio->query_fuels_basic_info(&bps_num, &defaultFBFM, &dryClimate);
}

void AnalysisPlot::push_shrub(RVS::DataManagement::DIO* dio, RVS::DataManagement::DataTable* dt)
{
	if (!dio || !dt)
	{
		return;
	}

	auto record = std::make_unique<RVS::DataManagement::SppRecord>(dio, dt);
	if (!record->DOM_SPP().empty())
	{
		shrubRecords.push_back(std::move(record));
	}
}

void AnalysisPlot::push_shrub(RVS::DataManagement::SppRecord* record)
{
	if (record)
	{
		shrubRecords.push_back(std::unique_ptr<RVS::DataManagement::SppRecord>(record));
	}
}

void AnalysisPlot::push_shrub(std::unique_ptr<RVS::DataManagement::SppRecord> record)
{
	if (record)
	{
		shrubRecords.push_back(std::move(record));
	}
}

void AnalysisPlot::update_shrubvalues()
{
	syncExternalShrubPointers();

	double totalCover = 0.0;
	double runShrubHeight = 0.0;
	double runShrubStem = 0.0;
	double totalShrubBiomass = 0.0;

	for (const auto& s : shrubRecords)
	{
		if (s)
		{
			totalCover += s->COVER();
			runShrubHeight += s->HEIGHT() * s->COVER();
			runShrubStem += s->SHRUB_SINGLE_BIOMASS() * s->COVER();
			totalShrubBiomass += s->SHRUB_EX_BIOMASS();
		}
	}

	shrubCover = totalCover;
	shrubBiomass = totalShrubBiomass;
	if (totalCover > 0.0)
	{
		shrubHeight = runShrubHeight / totalCover;
		shrubAvgStem = runShrubStem / totalCover;
	}
	else
	{
		shrubHeight = 0.0;
		shrubAvgStem = 0.0;
	}
}

double AnalysisPlot::getNDVI(std::string_view level, const bool useRand) const
{
	if (ndviValues.empty())
	{
		return 0.0;
	}

	double ndvi = 0.0;
	if (level == "Dry" && ndviValues.size() > 0)
	{
		ndvi = ndviValues[0];
	}
	else if (level == "Mid-Dry" && ndviValues.size() > 1)
	{
		ndvi = ndviValues[1];
	}
	else if (level == "Mid-Wet" && ndviValues.size() > 3)
	{
		ndvi = ndviValues[3];
	}
	else if (level == "Wet" && ndviValues.size() > 4)
	{
		ndvi = ndviValues[4];
	}
	else if (ndviValues.size() > 2)
	{
		ndvi = ndviValues[2];
	}
	else
	{
		ndvi = ndviValues[0];
	}

	if (useRand)
	{
		const int r = std::rand() % 15 + 90;
		ndvi = ndvi * (r / 100.0);
	}

	return ndvi;
}

double AnalysisPlot::getPPT(std::string_view level, const bool useRand) const
{
	if (precipValues.empty())
	{
		return 0.0;
	}

	double ppt = 0.0;
	if (level == "Dry" && precipValues.size() > 0)
	{
		ppt = precipValues[0];
	}
	else if (level == "Mid-Dry" && precipValues.size() > 1)
	{
		ppt = precipValues[1];
	}
	else if (level == "Mid-Wet" && precipValues.size() > 3)
	{
		ppt = precipValues[3];
	}
	else if (level == "Wet" && precipValues.size() > 4)
	{
		ppt = precipValues[4];
	}
	else if (precipValues.size() > 2)
	{
		ppt = precipValues[2];
	}
	else
	{
		ppt = precipValues[0];
	}

	if (useRand)
	{
		const int r = std::rand() % 15 + 90;
		ppt = ppt * (r / 100.0);
	}

	return ppt;
}

std::vector<RVS::Disturbance::DisturbAction> AnalysisPlot::getDisturbancesForYear(const int year) const
{
	std::vector<Disturbance::DisturbAction> yearDisturbances;
	for (const auto& d : disturbances)
	{
		if (d.getActionYear() == year)
		{
			yearDisturbances.push_back(d);
		}
	}
	return yearDisturbances;
}