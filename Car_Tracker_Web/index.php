<!DOCTYPE html>
<html style="height:100%;margin:0px;font-family: Verdana, sans-serif;">
    
<head>
	<?php include("config.php"); ?>
    <!-- Load Jquery -->

    <script src="//ajax.googleapis.com/ajax/libs/jquery/1.10.1/jquery.min.js"></script>

    <!-- Load Google Maps Api -->

    <!-- IMPORTANT: change the API v3 key -->

    <script src="<?php echo "http://maps.googleapis.com/maps/api/js?key=" . $google_maps_api_v3_key . "&sensor=false"; ?>"></script>

    <!-- Initialize Map and markers -->

    <script type="text/javascript">
        var myCenter=new google.maps.LatLng(<?php echo $default_lat; ?>,<?php echo $default_lng; ?>);
        var marker;
        var map;
        var mapProp;

        function initialize()
        {
            mapProp = {
              center:myCenter,
              zoom:15,
              mapTypeId:google.maps.MapTypeId.ROADMAP
              };
			  mark();
            //setInterval('mark()',5000);
        }

        function mark()
        {
            map=new google.maps.Map(document.getElementById("googleMap"),mapProp);
            var file = "gps.txt";
            $.get(file, function(txt) { 
                var lines = txt.split("\n");
				infoWindows = Array();
                for (var i=0;i<lines.length;i++){
                    console.log(lines[i]);
                    var words=lines[i].split(";");
                    if ((words[0]!="")&&(words[1]!=""))
                    {
                        marker=new google.maps.Marker({
                              position:new google.maps.LatLng(words[0],words[1]),
							  infoWindowIndex : i,
                        });
						
						var content = "Latitude: " + words[0] + "<br />" + 
									  "Longitude: " + words[1] + "<br />" +
									  "Date: " + words[2] + "<br />" +
									  "Time: " + words[3] + "<br />" +
									  "Altitude: " + words[4] + "<br />" +
									  "Speed: " + words[5] + "<br />";
        				
						var infoWindow = new google.maps.InfoWindow({
            				content : content
        				});
						
						google.maps.event.addListener(marker, 'click', function(event)
            			{
                			map.panTo(event.latLng);
                			infoWindows[this.infoWindowIndex].open(map, this);
            			});

        				infoWindows.push(infoWindow);
						
                        marker.setMap(map);
                        map.setCenter(new google.maps.LatLng(words[0],words[1]));
                        document.getElementById('date').innerHTML=words[2];
                        document.getElementById('time').innerHTML=words[3];
                        document.getElementById('alt').innerHTML=words[4];
						document.getElementById('speed').innerHTML=words[5];
                    }
                }
                marker.setAnimation(google.maps.Animation.BOUNCE);
            });

        }
		google.maps.visualRefresh = true;
        google.maps.event.addDomListener(window, 'load', initialize);
    </script>
</head>

<body style="height:100%;margin:0px;">
    <?php
        echo '    

        <!-- Draw information table and Google Maps div -->

        <div style="position:absolute;left:5%;top:5%;z-index:10;border:1px solid;background-color:#FFFFFF;padding-left:20px;padding-right:20px;padding-bottom:20px;">
                <h2 style="text-align:center;">Jeremy\'s Pod Tracker</h2>
				<em>Last Status Update:</em>
                <div id="superior" style="width:100%;">
                    <table style="width:100%">
                        <tr>
							<td><b>Date: </b></td><td id="date"></td>
						</tr><tr>
                            <td><b>Time: </b></td><td id="time"></td>
						</tr><tr>
							<td><b>Altitude: </b></td><td id="alt"></td>
						</tr><tr>
                            <td><b>Speed: </b></td><td id="speed"></td>
						</tr>
                	</table>
                </div>
        </div>
		<div id="googleMap" style="position:absolute;right:0px;bottom:0px;left:0px;top:0px"></div>';
    ?>
</body>
</html>