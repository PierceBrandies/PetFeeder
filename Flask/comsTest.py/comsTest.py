import requests

esp32_ip = '192.168.50.228:8081'

url = f'http://{esp32_ip}/'  # Include the port number in the URL
message = 'FEED'

response = requests.post(url, data={'message': message})
print(response.text)
