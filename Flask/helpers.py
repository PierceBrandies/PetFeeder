import cv2
from ultralytics import YOLO
import time
import threading
import requests
import urllib.request
import numpy as np

# Set Cam IP address
cam_ip = '192.168.50.36:8080'

# Connect python to cam webserver
def cam_connect():
  url = f'http://{cam_ip}/'
  message = 'Python connected to ESP32CAM'
  print("Attempting to connect to CAM")
  try:
    response = requests.post(url, data={'message': message})
    print(response.text)
    FLASHOFFresponse = requests.post(url, data={'message': 'FLASHOFF'})
    print(FLASHOFFresponse.text)
  except:
    print("Error: Could not connect to CAM.")


# Toggle camera flash
def flash(status):
  url = f'http://{cam_ip}/'
  if status == 'on' or status == 'ON':
    FLASHONresponse = requests.post(url, data={'message': 'FLASHON'})
    print(FLASHONresponse.text)
  else:
    FLASHOFFresponse = requests.post(url, data={'message': 'FLASHOFF'})
    print(FLASHOFFresponse.text)


# Check for feed delay params to handle if pet should be fed or not
def feed_pet(auto_feed, delay, portion):
  global timer_thread
  if auto_feed:
    if not check_timer_running(timer_thread):
        send_feed_command(portion)
        time.sleep(0.2)
        timer_thread = run_delay(delay)
    else:
      print("Timer active")
      return False
  else:
    send_feed_command(portion)
    

# Send command to rotate motor to feed pet
def send_feed_command(portion):
  url = f'http://{cam_ip}/'
  try:
    FEEDresponse = requests.post(url, data={'message': portion})
  except:
    print("Error: FEED command not sent.")
    return
  print(FEEDresponse.text)

  if portion == 'LARGE':
    time.sleep(8)
  elif portion == 'MED':
    time.sleep(6)
  else:
    time.sleep(4)


# Generate video stream from cam with object detetction
def generate_frames(delay, pet_id, accuracy, auto_feed, portion):
  # Load object detection model
  model = YOLO("yolov8n.pt")

  # Load video stream from ESP32 cam
  url = f'http://{cam_ip}/cam-mid.jpg'

  ret = True
  while ret:
    img_resp = urllib.request.urlopen(url)
    imgnp = np.array(bytearray(img_resp.read()), dtype = np.uint8)
    im = cv2.imdecode(imgnp, -1)

    detections = model(im)[0]
    for detection in detections.boxes.data.tolist():
      x1, y1, x2, y2, score, class_id = detection

      # If pet that has been selected is detected
      if int(class_id) == pet_id:
        x1, y1, x2, y2 = int(x1), int(y1), int(x2), int(y2)
        score = str(round(score, 2))

        # Display bounding box
        cv2.rectangle(im, (x1, y1), (x2, y2), (0, 255, 0), 3)
        # Display confidence score
        cv2.putText(im, score, (x1 + 40, y1 - 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 4)
        
        # If confidence score is >= selected accuracy setting
        if float(score) >= accuracy:
          if int(class_id) == 14:
            # Display Bird detected
            cv2.putText(im, "Bird Detected", (x1 + 40, y2 + 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 4)
          elif int(class_id) == 15:
            # Display Cat detected
            cv2.putText(im, "Cat Detected", (x1 + 40, y2 + 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 4)
          else:
            # Display Binnie detected
            cv2.putText(im, "Binnie Detected", (x1 + 40, y2 + 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 4)
          print("FOUND")

          if auto_feed:
            if not feed_pet(auto_feed, delay, portion):
              cv2.putText(im, "Feed-delay Active", (x1 + 40, y2 + 80), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 4)
            else:
              cv2.putText(im, "Feeding", (x1 + 40, y2 + 80), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 4)
            
    # Convert frame to JPEG format then into bytes
    ret, buffer = cv2.imencode('.jpg', im)
    im = buffer.tobytes()

    # Yield a bytestring for HTTP streaming
    yield (b'--frame\r\n'
            b'Content-Type: image/jpeg\r\n\r\n' + im + b'\r\n')
    

# Convert delay from seconds, minutes, hours, to seconds
def calc_delay(seconds, minutes, hours):
    try:
      seconds = int(seconds)
    except:
      seconds = 0
    try:
      minutes = int(minutes)
    except:
      minutes = 0
    try:
      hours = int(hours)
    except:
      hours = 0

    minutes = (minutes * 60)
    hours = (hours * 3600)

    total_delay = seconds + minutes + hours
    return total_delay


# Countdown timer
def countdown_function(total_delay):
    counter = total_delay
    while counter > 0:
        print(f"Time left: {counter} seconds")
        counter -= 1
        time.sleep(1)
    print("Timer finished")
    return False


# Run timer in a seperate thread
def run_delay(total_delay):
    timer_thread = threading.Thread(target=countdown_function, args=(total_delay,))
    timer_thread.daemon = True
    timer_thread.start()
    time.sleep(0.1)
    return timer_thread


# Check timers active status and return a boolean
def check_timer_running(timer_thread):
    if timer_thread.is_alive():
        print("Timer still running")
        return True
    else:
        print("Timer finished")
        return False


# Initiate a timer thread
timer_thread = run_delay(5)