from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen
import re

country_list_file = "worldometers_countrylist.txt"
base_url = "https://www.worldometers.info/coronavirus/"
webpage_extension = ".html"
index_page = "index.html"
website_directory = "webpages/"
continent_dict = {}
file_to_web_map = {
    "North America": "north_america",
    "Europe": "europe",
    "Asia": "asia",
    "South America": "south_america",
    "Africa": "africa",
    "Oceania": "oceania",
    "France": "france",
    "UK": "uk",
    "Russia": "russia",
    "Italy": "italy",
    "Germany": "germany",
    "Spain": "spain",
    "Poland": "poland",
    "Netherlands": "netherlands",
    "Ukraine": "ukraine",
    "Belgium": "belgium",
    "USA": "us",
    "Mexico": "mexico",
    "Canada": "canada",
    "Cuba": "cuba",
    "Costa Rica": "costa-rica",
    "Panama": "panama",
    "India": "india",
    "Turkey": "turkey",
    "Iran": "iran",
    "Indonesia": "indonesia",
    "Philippines": "philippines",
    "Japan": "japan",
    "Israel": "israel",
    "Malaysia": "malaysia",
    "Thailand": "thailand",
    "Vietnam": "viet-nam",
    "Iraq": "iraq",
    "Bangladesh": "bangladesh",
    "Pakistan": "Pakistan",
    "Brazil": "brazil",
    "Argentina": "argentina",
    "Colombia": "colombia",
    "Peru": "peru",
    "Chile": "chile",
    "Bolivia": "bolivia",
    "Uruguay": "uruguay",
    "Paraguay": "paraguay",
    "Venezuela": "venezuela",
    "South Africa": "south-africa",
    "Morocco": "morocco",
    "Tunisia": "tunisia",
    "Ethiopia": "ethiopia",
    "Libya": "libya",
    "Egypt": "egypt",
    "Kenya": "kenya",
    "Zambia": "zambia",
    "Algeria": "algeria",
    "Botswana": "botswana",
    "Nigeria": "nigeria",
    "Zimbabwe": "zimbabwe",
    "Australia": "australia",
    "Fiji": "fiji",
    "Papua New Guinea": "papua-new-guinea",
    "New Caledonia": "new-caledonia",
    "New Zealand": "new-zealand"
}


def find_continent(country_name=""):
    for continent, country_list in continent_dict.items():
        for country in country_list:
            if country_name == country:
                return continent
    return None


def save_webpage(webpage_content, country_location):
    global website_directory
    if country_location == website_directory:
        page_location = country_location + index_page
    else:
        page_location = country_location
    f = open(page_location, 'w')
    f.write(webpage_content)
    f.close()


def download_webpage(country=""):
    global base_url
    country_location = website_directory
    url = base_url
    if country:
        url = base_url + "country/" + file_to_web_map[country] + "/"
        continent = file_to_web_map[find_continent(country)]
        country_location = website_directory + continent + "/" + file_to_web_map[country] + ".html"

    try:
        request = Request(url, headers={'User-Agent': 'Mozilla/5.0'})
        webpage_content = urlopen(request).read().decode('utf-8')
    except HTTPError as e:
        print("Not able to reach WorldoMeters Website. Please check you internet connection.")
        exit(0)
    except URLError as e:
        print("Not able to reach WorldoMeters Website. Please check you internet connection.")
        exit(0)
    except Exception as e:
        print("Not able to reach WorldoMeters Website. Please check you internet connection.")
        exit(0)

    print(f'Webpage Downloaded from {url}')
    save_webpage(webpage_content, country_location)


def download_website(is_download_statistics):
    global continent_dict
    global file_to_web_map
    # Creating a dictionary of continents with values as list of countries provided in country_list_file
    country_file = open(country_list_file, "r")
    continent = ""
    cnt_list = []

    index = 0
    for line in country_file:
        result = re.search("[a-zA-z ].*:", line)
        if result:
            # Creating a dictionary of continents with values as list of countries
            if index != 0:
                continent = continent[:-2]
                continent_dict[continent] = cnt_list
            append = 0
            continent = line
            cnt_list = []
        else:
            append = 1

        # Creating a list of countries for a given continent
        if append == 1 and line != "---------\n" and line != "\n":
            cnt_list.append(line.replace("\n", ""))
        index += 1
    # adding the last item in the dict
    continent = continent[:-2]
    continent_dict[continent] = cnt_list

    country_file.close()

    # Download Index page
    if is_download_statistics:
        download_webpage()

    # Download the country pages
    for continent, country_list in continent_dict.items():
        if is_download_statistics:
            print(f'Downloading webpages of {continent} ...')
        for country in country_list:
            if is_download_statistics:
                download_webpage(country)
    print(f'Parsing Data ...')