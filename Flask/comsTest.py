import requests

# Set Cam IP address
cam_ip = 'YOUR_CAM_IP_ADDRESS_HERE:PORT_NUMBER_HERE'

url = f'http://{cam_ip}/'
message = 'TEST'

response = requests.post(url, data={'message': message})
print(response.text)