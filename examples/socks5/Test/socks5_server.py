import requests

host_url = 'https://example.com'
# Fill in your own proxies' details
proxies = {http: 'socks5://user:pass@host:port',
           https: 'socks5://user:pass@host:port'}
# define headers if you will
headers = {}

response = requests.get(host_url, headers=headers, proxies=proxies)
