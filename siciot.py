import serial
import time
import requests

# Thông tin ThingSpeak
THINGSPEAK_API_KEY = 'K0I15TSJE6PQLY44'  # Thay bằng Write API Key của bạn
THINGSPEAK_URL = 'https://api.thingspeak.com/update'

# Mở cổng Serial (sử dụng cổng mà bạn tìm thấy ở bước trước)
ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)  # Thay đổi '/dev/ttyUSB0' nếu cần thiết
ser.flush()


def send_to_thingspeak(temp_water, temp_air, humidity, pH_value):
  payload = {'api_key': THINGSPEAK_API_KEY, 'field1': temp_water,  # Đẩy nhiệt độ nước lên field1
    'field2': temp_air,  # Đẩy nhiệt độ không khí lên field2
    'field3': humidity,  # Đẩy độ ẩm lên field3
    'field4': pH_value  # Đẩy giá trị pH lên field4
  }
  try:
    response = requests.get(THINGSPEAK_URL, params=payload)
    return response
  except Exception as e:
    print(f"Failed to send data to ThingSpeak due to error: {e}")
    return None


while True:
  if ser.in_waiting > 0:
    try:
      # Đọc dữ liệu từ Arduino
      line = ser.readline().decode('utf-8').rstrip()
      print(f"Received raw data: {line}")

      # Phân tích dữ liệu nhận được
      data = line.split(',')
      print(f"Parsed data: {data}")

      if len(data) == 4:
        try:
          temp_water = float(data[0])  # Nhiệt độ nước
          temp_air = float(data[1])  # Nhiệt độ không khí
          humidity = float(data[2])  # Độ ẩm
          pH_value = float(data[3])  # Giá trị pH

          # Gửi dữ liệu lên ThingSpeak
          response = send_to_thingspeak(temp_water, temp_air, humidity, pH_value)
          if response and response.status_code == 200:
            print("Data sent to ThingSpeak successfully!")
          else:
            print(f"Failed to send data to ThingSpeak. Status code: {response.status_code}")
        except ValueError:
          print("Received non-numeric data from Arduino.")
      else:
        print("Unexpected data format received.")
    except Exception as e:
      print(f"Error processing data: {e}")

  # Thêm delay trước khi gửi lần tiếp theo (15 giây theo giới hạn của ThingSpeak)
  time.sleep(15)
