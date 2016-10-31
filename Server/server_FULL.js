var DEMO_MODE = true;

var TIME_SCALE = 1; // calibrate prediction (e.g. TIME_SCALE=1.4 will give a more accurate prediction)

// helper function to execute system commands (needed for "curl" command)
var exec = require('child_process').exec;

// helper function to parse XML to JSON
var parseXML2JS = require('xml2js').parseString;


var time_estimate_minutes = 11;

// Express web server
var express = require('express');
var app = express();

app.get('/', function(req, res) {
    res.send('Transit Notifier Server');
    //hi
});

app.get('/trolley_to_HUB_stop_Klaus', function(req, res) {
    res.send('' + time_estimate_minutes);
});

app.listen(3000, function() {
    console.log('Transit Notifier Server listening on port 3000!');
});




if (DEMO_MODE) {

    setInterval(function() {


        if (time_estimate_minutes == 0) {
            time_estimate_minutes = 15;
        }
        --time_estimate_minutes;
        console.log("[DEMO] prediction: " + time_estimate_minutes + " min");

    }, 1200); // 1200 is scaling by 1/50th i.e. 1 minute is 1.2 seconds


} else {

    setInterval(function() {
        // request the trolley predictions to find the "closest" trolley going towards the HUB
        CURL("https://gtbuses.herokuapp.com/predictions/trolley", function(stdout, stderr) {

            // // print CURL response and debug information
            // console.log('stdout: ' + stdout);
            // console.log('stderr: ' + stderr);

            var xml = stdout;

            parseXML2JS(xml, function(err, result) {
                if (err) {
                    console.error("xml2js error: " + err);
                    return;
                }
                // // print xml2json result
                // console.dir(result.body.predictions);

                for (var i = 0; i < result.body.predictions.length; ++i) {
                    if (result.body.predictions[i].$.stopTag == 'ferschrec') {

                        var date = new Date();
                        console.log(date);

                        // get only prediction[0]. That is the trolley arriving the soonest
                        console.log("NextBus Prediction: " + result.body.predictions[i].direction[0].prediction[0].$.seconds + " sec");
                        console.log("NextBus Prediction: " + result.body.predictions[i].direction[0].prediction[0].$.minutes + " min");
                        console.log("Bus ID: " + result.body.predictions[i].direction[0].prediction[0].$.vehicle);

                        time_estimate_minutes = TIME_SCALE * result.body.predictions[i].direction[0].prediction[0].$.minutes;
                    }
                }
            });
            /*
                // google map predictions can be obtained with:
                CURL("https://maps.googleapis.com/maps/api/directions/json?key=" + GOOGLEMAPS_API_KEY + "&origin=" + bus_lat + "," + bus_long + "&destination=33.776993,-84.395285", function(stdout, stderr) {

                    console.log('stdout: ' + stdout);
                    console.log('stderr: ' + stderr);

                });
            */
        });
    }, 1000);


}








function CURL(url, end_callback) {

    function exec_callback(error, stdout, stderr) {
        if (error !== null) {
            console.error('exec error: ' + error);
            return;
        }

        end_callback(stdout, stderr);
    }
    exec("curl \"" + url + "\"", exec_callback);
}
