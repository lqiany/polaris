.separator ","
DELETE FROM ZoneLandUse;
.import z:/POLARIS/interchange/DemandData/ZoneLandUse ZoneLandUse

DELETE FROM LocationData;
.import z:/POLARIS/interchange/DemandData/LocationData LocationData
--For some reason the UPDATE statement would not wor without sqlite extension being loaded
SELECT load_extension('c:/opt/polarisdeps/Win32/bin/libspatialite-4.dll');
UPDATE Zone SET zone_land_use = (SELECT zone FROM ZoneLandUse WHERE ZoneLandUse.zone = Zone.zone);
UPDATE Location SET location_data = (SELECT location FROM LocationData WHERE LocationData.location = Location.location);