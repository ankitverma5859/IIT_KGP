from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen
import re

base_url = "https://en.wikipedia.org"
index_path = "/wiki/Timeline_of_the_COVID-19_pandemic"
website_directory = "covid_wiki/"
index_page = "index.html"
country_map = {
    "Argentina": "Argentina",
    "Australia": "Australia",
    "Bangladesh": "Bangladesh",
    "Brazil": "Brazil",
    "Canada": "Canada",
    "Ghana": "Ghana",
    "India": "India",
    "Indonesia": "Indonesia",
    "the_Republic_of_Ireland": "Ireland",
    "Malaysia": "Malaysia",
    "Mexico": "Mexico",
    "New_Zealand": "New_Zealand",
    "Nigeria": "Nigeria",
    "Pakistan": "Pakistan",
    "the_Philippines": "Philippines",
    "Russia": "Russia",
    "Singapore": "Singapore",
    "South_Africa": "South_Africa",
    "Spain": "Spain",
    "Turkey": "Turkey",
    "England": "England",
    "the_United_States": "United_States"
}


def extract_country(url):
    country_name = re.search("_in_[a-zA-Z_]*", url)
    country_name = country_name.group().replace("_in_", "")
    if country_name[len(country_name) - 1] == "_":
        country_name = country_name[:-1]
    return country_name


def save_webpage(webpage_content, location):
    global website_directory
    if location == website_directory:
        page_location = location + index_page
    else:
        page_location = location
    f = open(page_location, 'w')
    f.write(webpage_content)
    f.close()


def download_webpage(url_page, page_type):
    global base_url
    page_location = website_directory

    if page_type == "timeline":
        splitted_url = url_page.split("_")
        if len(splitted_url) == 7:
            page_location = page_location + "timeline/" + splitted_url[6] + ".html"
        else:
            page_location = page_location + "timeline/" + splitted_url[6] + "_" + splitted_url[7] + ".html"
    elif page_type == "responses":
        splitted_url = url_page.split("_")
        page_location = page_location + "responses/" + splitted_url[6] + "_" + splitted_url[7] + ".html"
    elif page_type == "country":
        replace_strings = ["%E2%80%93", "(", ")"]
        page_name = re.search("_in_[a-zA-Z_()%0-9]*", url_page)
        page = page_name.group().replace("_in_", "")
        for replace_str in replace_strings:
            if replace_str == "%E2%80%93":
                page = page.replace(replace_str, "_")
            else:
                page = page.replace(replace_str, "")
        country_name = extract_country(url_page)
        page_location = page_location + "country/" + country_map[country_name] + "/" + page + ".html"

    try:
        request = Request(url_page, headers={'User-Agent': 'Mozilla/5.0'})
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

    print(f'Webpage Downloaded from {url_page}')
    save_webpage(webpage_content, page_location)


def download_website():
    timeline_urls = []
    response_urls = []
    country_urls = []

    # download index webpage
    download_webpage(base_url + index_path, "index")

    # read the index page content
    covid_page = open('covid_wiki/index.html', 'r')
    covid_page_data = covid_page.read()

    # extract the timeline urls from index page
    regex_timeline_url_path = "(\/wiki\/Timeline_of_the_COVID-19_pandemic_in_+(?:2019|(January|February|March|April|May|June|July|August|September|October|November|December)+(_2020|_2021|_2022)))"
    timeline_url_path_matches = re.findall(regex_timeline_url_path, covid_page_data)
    for url_path in timeline_url_path_matches:
        url_page = base_url + url_path[0]
        download_webpage(url_page, "timeline")
        timeline_urls.append(url_page)

    # extract the response urls from index page
    regex_response_url_path = "(\/wiki\/Responses_to_the_COVID-19_pandemic_in_+(January|February|March|April|May|June|July|August|September|October|November|December)+(_2020|_2021|_2022))"
    response_url_path_matches = re.findall(regex_response_url_path, covid_page_data)
    for url_path in response_url_path_matches:
        url_page = base_url + url_path[0]
        download_webpage(url_page, "responses")
        response_urls.append(url_page)

    regex_country_url_path = "(\/wiki\/Timeline_of_the_COVID-19_pandemic_in_+(Argentina|Australia|Bangladesh|Brazil|Canada|Ghana|India|Indonesia|the_Republic_of_Ireland|Malaysia|Mexico|New_Zealand|Nigeria|Pakistan|the_Philippines|Russia|Singapore|South_Africa|Spain|Turkey|England|the_United_States)+[_\(a-zA-Z%0-9\)]*)"
    country_url_path_matches = re.findall(regex_country_url_path, covid_page_data)
    for url_path in country_url_path_matches:
        url_page = base_url + url_path[0]
        download_webpage(url_page, "country")
        country_urls.append(url_page)

















