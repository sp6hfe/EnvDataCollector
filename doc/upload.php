<?php

$db_host = "localhost";
$db_user = "envdatacollector";
$db_pass = "rE1-hXXbu9";
$db_name = "sp6hfe";
$api_upload_key = "667F6C3E9D911C79B0806E5F88D34C57";

$sensor_id = $api_key = $token = $temperature = $humidity = $pressure_raw = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
	// retrieve parameters
	$api_key = test_input($_POST["api_key"]);
	$token = test_input($_POST["token"]);
	$temperature = test_input($_POST["temperature"]);
	$humidity = test_input($_POST["humidity"]);
	$pressure_raw = test_input($_POST["pressure_raw"]);

    if($api_key == $api_upload_key) {
		// create DB connection
        $db = new mysqli($db_host, $db_user, $db_pass, $db_name);
		if ($db->connect_error) {
			echo "DB connection failed: " . $db->connect_error . ".";
			exit();
		}

		// get sensor id based on token provided
		$sql_select = "SELECT id, token FROM sensor WHERE token = '" . $token . "'";
        $select_result = $db->query($sql_select);
		if ($select_result) {
			if ($result_row = $select_result->fetch_row()) {
				$sensor_id = $result_row[0];
			}
		}

		// insert new data for particular sensor
        if ($sensor_id) {
			$sql_insert = "INSERT INTO data (sensor_id, temperature, humidity, pressure_raw) VALUES ('" . $sensor_id . "', '" . $temperature . "', '" . $humidity . "', '" . $pressure_raw . "')";
			if ($db->query($sql_insert) === TRUE) {
				echo "Data saved.";
			} 
			else {
				echo "Error saving data: " . $db->error . ".";
			}
		}
		
		$db->close();
    }
    else {
        echo "Wrong API key.";
    }
}
else {
    echo "No POST data received.";
}

?>