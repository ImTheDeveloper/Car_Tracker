<?php
	include("config.php");
	//Confirm that that the request is coming from my twilio account.
	if (isset($_POST['AccountSid']) && isset($_POST['From']) && isset($_POST['Body']))
	{
		if ($_POST['AccountSid'] == $twilio_sid)
		{
			//Format the GPS Data. Assume always in Northern/Western Hemispheres.
			$GPS_raw = explode(",",$_POST['Body']);
			$lat = substr($GPS_raw[0],0,2) + (substr($GPS_raw[0],2)/60.0);
			$lng = -1 *(substr($GPS_raw[2],0,3) + (substr($GPS_raw[2],3)/60.0));
			
			$month = substr($GPS_raw[4],2,2);
			$day = substr($GPS_raw[4],0,2);
			$year = substr($GPS_raw[4],4,2);
			$date = date("F", mktime(0, 0, 0, $month, 10)) . " " . $day . ", 20" . $year;
			
			$utc_hour = substr($GPS_raw[5],0,2);
			$utc_min = substr($GPS_raw[5],2,2);
			$time = $utc_hour . ":" . $utc_min . " UTC";
			
			$alt = $GPS_raw[6];
			$speed = $GPS_raw[7];
			
			$file = 'gps.txt';
			$data = $lat . ";" . $lng . ";" . $date . ";" . $time . ";" . $alt . ";" . $speed . "\n";
			file_put_contents($file, $data, FILE_APPEND | LOCK_EX);
		}
	}
?>