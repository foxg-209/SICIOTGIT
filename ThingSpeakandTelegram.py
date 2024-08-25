import serial
import time
import requests
from telegram.ext import Updater, MessageHandler, Filters

# Thông tin ThingSpeak
THINGSPEAK_API_KEY = 'K0I15TSJE6PQLY44'  # Thay bằng Write API Key của bạn
THINGSPEAK_URL = 'https://api.thingspeak.com/update'

# Thông tin Telegram Bot
TELEGRAM_BOT_TOKEN = '7108158962:AAFbwa-quvDHgK5zswKGLlUbXS5L_iOpFFE'  # Thay bằng token của bot Telegram của bạn
TELEGRAM_CHAT_ID = '-4579290375'  # Thay bằng chat ID hoặc channel ID

# Mở cổng Serial (sử dụng cổng mà bạn tìm thấy ở bước trước)
ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)  # Thay đổi '/dev/ttyUSB0' nếu cần thiết
ser.flush()

def send_to_thingspeak(temp_water, temp_air, humidity, pH_value):
    payload = {
        'api_key': THINGSPEAK_API_KEY,
        'field1': temp_water,  # Đẩy nhiệt độ nước lên field1
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

def send_telegram_alert(message):
    url = f"https://api.telegram.org/bot{TELEGRAM_BOT_TOKEN}/sendMessage"
    payload = {'chat_id': TELEGRAM_CHAT_ID, 'text': message}
    try:
        requests.post(url, data=payload)
        print("Alert sent to Telegram.")
    except Exception as e:
        print(f"Failed to send alert to Telegram: {e}")

def handle_message(update, context):
    text = update.message.text.lower()
    chat_id = update.message.chat_id
    if text == "gui" and chat_id == int(TELEGRAM_CHAT_ID):
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            data = line.split(',')
            if len(data) == 4:
                try:
                    temp_water = float(data[0])
                    temp_air = float(data[1])
                    humidity = float(data[2])
                    pH_value = float(data[3])
                    message = (f"Nhiệt độ nước: {temp_water}°C\n"
                               f"Nhiệt độ không khí: {temp_air}°C\n"
                               f"Độ ẩm: {humidity}%\n"
                               f"Giá trị pH: {pH_value}")
                    context.bot.send_message(chat_id=TELEGRAM_CHAT_ID, text=message)
                except ValueError:
                    context.bot.send_message(chat_id=TELEGRAM_CHAT_ID, text="Received non-numeric data from Arduino.")
            else:
                context.bot.send_message(chat_id=TELEGRAM_CHAT_ID, text="Unexpected data format received.")
        else:
            context.bot.send_message(chat_id=TELEGRAM_CHAT_ID, text="No data available.")

# Khởi tạo bot và gán MessageHandler
updater = Updater(TELEGRAM_BOT_TOKEN, use_context=True)
dp = updater.dispatcher
dp.add_handler(MessageHandler(Filters.text & ~Filters.command, handle_message))

updater.start_polling()

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

                    # Kiểm tra nhiệt độ nước và gửi cảnh báo nếu cần thiết
                    alerts = []
                    if temp_water < 20 or temp_water >25:
                        alerts.append(f"Cảnh báo: Nhiệt độ nước cao: {temp_water}°C!")
                    if pH_value < 7 or pH_value > 7.5:
                        alerts.append(f"Cảnh báo: Độ pH cao: {pH_value}!")

                    if alerts:
                        for alert in alerts:
                            send_telegram_alert(alert)

                except ValueError:
                    print("Received non-numeric data from Arduino.")
            else:
                print("Unexpected data format received.")
        except Exception as e:
            print(f"Error processing data: {e}")

    # Thêm delay trước khi gửi lần tiếp theo (15 giây theo giới hạn của ThingSpeak)
    time.sleep(15)