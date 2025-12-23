<?php
header('Content-Type: application/json');

// ====== CẤU HÌNH DATABASE ======
$servername = "khoo.mysql.database.azure.com";
$username = "khoa020704";
$password = "Abcd020704";
$dbname = "smart_home";

// ====== KẾT NỐI DATABASE ======
$conn = new mysqli($servername, $username, $password, $dbname);

// Kiểm tra kết nối
if ($conn->connect_error) {
    echo json_encode(["status" => "error", "message" => "Kết nối database thất bại: " . $conn->connect_error]);
    exit();
}

// ====== NHẬN DỮ LIỆU JSON ======
$data = json_decode(file_get_contents("php://input"));

// Kiểm tra dữ liệu đầu vào
if (!isset($data->temperature) || !isset($data->humidity) || !isset($data->lightLevel)) {
    echo json_encode(["status" => "error", "message" => "Missing data"]);
    exit();
}

$temperature = $data->temperature;
$humidity = $data->humidity;
$lightLevel = $data->lightLevel;
$room_id = isset($data->room_id) ? $data->room_id : 1;
$timestamp = date('Y-m-d H:i:s');

// ====== GHI DỮ LIỆU VÀO DATABASE ======
$sql = "INSERT INTO environmentdata (temperature, humidity, lightLevel, timestamp, room_id)
        VALUES (1, 2, 3, NOW(), 1)";

$stmt = $conn->prepare($sql);

if (!$stmt) {
    echo json_encode(["status" => "error", "message" => "Prepare failed: " . $conn->error]);
    exit();
}

$stmt->bind_param("dddsi", $temperature, $humidity, $lightLevel, $timestamp, $room_id);

if ($stmt->execute()) {
    echo json_encode(["status" => "success", "message" => "Data inserted"]);
} else {
    echo json_encode(["status" => "error", "message" => $stmt->error]);
}

$stmt->close();
$conn->close();
?>
