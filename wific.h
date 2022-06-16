
#include <swifi.h>

void InitWifi()
{
  SWifi::SetAP(HOSTNAME, "thomewifijelszo");
  SWifi::AddWifi("wifi1", "12345678");
  SWifi::AddWifi("wifi2", "12345678");
  SWifi::SetWifiMode(true, true);
  SWifi::SetHostname(HOSTNAME);
  SWifi::Connect();
  SWifi::InitOTA(); //to enable OTA updates
}


void handleAdc()
{
  int x = analogRead(35);
  String a = String(x);
  server.send(200, "text/plain", "ADC value:" + a + "<br/>Battery: " + String(ReadBattery())); 
}

void handleRoot()
{
  server.send(200,"text/html", "<html><body><a href='/gps'>GPS</a><br/><a href='/sensi'>sensitivity</a><br/><a href='/adc'>Adc</a><br/><a href='/sens'>Sensitivity</a><br/></body></html>");
}
void handleSensi()
{
  server.send(200,"text/html", "<html><body><a href='/gps'>GPS</a><br/><a href='/sensi'>sensitivity</a><br/><a href='/adc'>Adc</a><br/><a href='/sens'>Sensitivity</a><br/></body></html>");
}


void handleGps()
{
  String html = "<html><head><title>TCar</title><meta charset=\"utf-8\"><script src=\"https://cdn.rawgit.com/openlayers/openlayers.github.io/master/en/v5.3.0/build/ol.js\"></script><link rel=\"stylesheet\" href=\"https://cdn.rawgit.com/openlayers/openlayers.github.io/master/en/v5.3.0/css/ol.css\"></script><script src=\"https://cdn.polyfill.io/v2/polyfill.min.js?features=requestAnimationFrame,Element.prototype.classList,URL\"></script></head><body>GPS:";
  html += String(gpsLatitude,6);
  html += "   " + String(gpsLongitude,6) + "\n";
  html += "Speed: " + String(gpsSpeed,6) + " kmh\n ";
  html += "Altitude: " + String(gpsAltitude,1) + " m. " + "Satelites: " + String(gpsUSat) + ".  Accuracy: " + String(gpsAccuracy,1);
  html += "<div id=\"map\" style=\"height:550px\"></div><script>function minutes_to_decimal(minutes){ return (minutes / 60);}";
  html += " function get_longitude() { return("+String(gpsLongitude,6)+") ;} ";
  html += "function get_latitude() { return "+String(gpsLatitude,6)+";}";
  html += "var lon = get_longitude();var lat = get_latitude();var position  = ol.proj.fromLonLat([lon, lat]);marker = new ol.Feature({ geometry: new ol.geom.Point(position) });var markers = new ol.source.Vector({});markers.addFeature(marker);var markerVectorLayer = new ol.layer.Vector({source: markers, title: 'Markers'});view = new ol.View({ center: position, zoom: 15 });var mapnik = new ol.layer.Tile({ baseLayer: true, title:'Base',  source: new ol.source.OSM()  });";
  html += "map = new ol.Map( { view: view, target: 'map', layers:[mapnik, markerVectorLayer] });view.animate({zoom: 15, center: position});</script></body></html>";
  server.send(200,"text/html", html);
}


void InitWebServer()
{
  server.on("/adc", handleAdc); //to support resistance based remotes in the future
  server.on("/", handleRoot);
  server.on("/gps", handleGps); //show current pos + details in map
  server.on("/sensi", handleSensi); //todo future sensitivity settings
  server.begin();    
}
