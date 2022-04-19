import re
import common
import download_website

country_graphs = {}


def print_options():
    print(
        '''
        1.\tTotal Cases
        2.\tActive Cases
        3.\tTotal Deaths
        4.\tTotal Recovered
        5.\tTotal Tests
        6.\tDeath/million
        7.\tTests/million
        8.\tNew Cases
        9.\tNew Deaths
        10.\tNew Recovered
        '''
    )


def print_continents():
    s_no = 1
    for continent in download_website.continent_dict.keys():
        print(f'\t\t{s_no}.\t{continent}')
        s_no += 1


def print_countries():
    itr = 1
    for continent, countries in download_website.continent_dict.items():
        print(f'{continent}:')
        for country in countries:
            print(f'\t{itr}. {country}')
            itr += 1


def get_cases_key(option_number):
    switcher = {
        1: "total_cases",
        2: "active_cases",
        3: "total_deaths",
        4: "total_recovered",
        5: "total_tests",
        6: "deaths_per_million",
        7: "tests_per_million",
        8: "new_cases",
        9: "new_deaths",
        10: "new_recovered",
    }
    return switcher.get(int(option_number), None)


def get_continents_key(option_number):
    switcher = {
        1: "Europe",
        2: "North America",
        3: "Asia",
        4: "South America",
        5: "Africa",
        6: "Oceania",
    }
    return switcher.get(int(option_number), None)


def get_country_name(country_number):
    itr = 1
    for continent, country_list in download_website.continent_dict.items():
        for country in country_list:
            if itr == country_number:
                return country
            itr += 1


def extract_stat(stat):
    if stat == '<td>N/A</td>':
        return 'N/A'
    extracted_stat = re.search(r'>[a-zA-Z 0-9,\.\+]*<', stat)
    return extracted_stat.group()[1:-1]


def extract_country_name(country):
    if country == 'N/A':
        return
    if "span" in country:
        s1 = re.sub("<span>", "", country)
        s2 = re.sub("</span>", "", s1)
        s3 = re.sub("<td>", "", s2)
        s4 = re.sub("</td>", "", s3)
        return s4
    else:
        s1 = re.sub("<td>", "", country)
        s2 = re.sub("</td>", "", s1)
        s3 = re.sub(r'<a(\s)class="mt_a"(\s)href="[a-zA-Z -\/]+">', "", s2)
        s4 = re.sub(r"</a>", "", s3)
        return s4


def extract_country_graphs():
    for continent, countries in download_website.continent_dict.items():
        for country in countries:
            country_name = download_website.file_to_web_map[country]
            continent = download_website.file_to_web_map[download_website.find_continent(country)]
            webpage_location = "webpages/" + continent + "/" + country_name + ".html"
            extract_graph_data(webpage_location, country)


def extract_graph_data(country_location, country_name):
    f = open(country_location, 'r')
    data = f.read()
    f.close()

    graph_stats = {}

    # Active Cases Parsing
    active_cases_script = re.search(r'<div id=\"graph-active-cases-total\"><\/div>(\n)*<script type="text\/javascript">'
                                    r'[a-zA-Z0-9 :\n\s\.()\'\-,{}\[\]"\#;<>\+\$\/=]+'
                                    r'<\/script>', data)
    if active_cases_script != None:
        #print('Active Cases:')
        active_cases_str = active_cases_script.group()
        #print(active_cases_str)

        #   Extracting the dates
        date_range = re.search(r'categories:(\s)[a-zA-z0-9\"\n\s,]+]', active_cases_str)
        date_range = re.sub('categories:(\s)', '', date_range.group())
        date_list = date_range[2:-2].split('","')
        for itr, date in enumerate(date_list):
            date_list[itr] = date.replace(",", "")
        #print(f'Dates: {date_list}')

        #   Extracting the values
        values = re.search(r'data:(\s)[a-zA-z0-9\"\n\s,-]+]', active_cases_str)
        values = re.sub('data:(\s)', '', values.group())
        values = values[1:-1].split(',')
        for itr, value in enumerate(values):
            values[itr] = int(value)
        #print(f'Values: {values}')

        # Creating dictionary of date: value for active cases
        active_cases = {}
        for itr, date in enumerate(date_list):
            active_cases[date] = values[itr]
        #print(f'Active Cases: {active_cases}')
        graph_stats['active_cases'] = active_cases


    # Daily Deaths Parsing
    daily_deaths_script = re.search(r'<div id=\"graph-deaths-daily\"><\/div>(\n)*<script type="text\/javascript">'
                   r'[a-zA-Z0-9 :\n\s\.()\'\-,{}\[\]"\#;<>\+\$\/=]+'
                   r'<\/script>', data)
    if daily_deaths_script != None:
        #print('Daily Deaths:')
        daily_deaths_str = daily_deaths_script.group()
        # print(daily_deaths_str)

        #   Extracting the dates
        date_range = re.search(r'categories:(\s)[a-zA-z0-9\"\n\s,]+]', daily_deaths_str)
        date_range = re.sub('categories:(\s)', '', date_range.group())
        date_list = date_range[2:-2].split('","')
        for itr, date in enumerate(date_list):
            date_list[itr] = date.replace(",", "")
        #print(f'Dates: {date_list}')

        #   Extracting the values
        values = re.search(r'data:(\s)[a-zA-z0-9\"\n\s,-]+]', daily_deaths_str)
        values = re.sub('data:(\s)', '', values.group())
        values = values[1:-1].split(',')
        for itr, value in enumerate(values):
            if value == 'null':
                values[itr] = 0
            else:
                values[itr] = int(value)
        #print(f'Values: {values}')

        # Creating dictionary of date: value for active cases
        daily_deaths = {}
        for itr, date in enumerate(date_list):
            daily_deaths[date] = values[itr]
        #print(f'Daily Deaths: {daily_deaths}')
        graph_stats['daily_deaths'] = daily_deaths

    # New Recovery and New Cases parsing
    new_recoveries_and_cases_script = re.search(r'<div id=\"cases-cured-daily\"><\/div>(\n)*<script type="text\/javascript">'
                                    r'[a-zA-Z0-9 :\n\s\.()\'\-,{}\[\]"\#;<>\+\$\/=]+'
                                    r'<\/script>', data)

    if new_recoveries_and_cases_script != None:
        #print('New Recoveries and Cases:')
        new_recoveries_and_cases_str = new_recoveries_and_cases_script.group()
        # print(new_recoveries_and_cases_str)

        #   Extracting the dates
        date_range = re.search(r'categories:(\s)[a-zA-z0-9\"\n\s,]+]', new_recoveries_and_cases_str)
        date_range = re.sub('categories:(\s)', '', date_range.group())
        date_list = date_range[2:-2].split('","')
        for itr, date in enumerate(date_list):
            date_list[itr] = date.replace(",", "")
        # print(f'Dates: {date_list}')

        # Extracting new recovery values
        new_recoveries_and_cases_str_without_newlines = re.sub("\n", "", new_recoveries_and_cases_str)
        new_recoveries_and_cases_str_without_spaces = ' '.join(new_recoveries_and_cases_str_without_newlines.split())
        new_recovery_values = re.search(
            r'name:(\s)\'New Recoveries\',(\s)color:(\s)\'\#8ACA2B\',(\s)lineWidth:(\s)5,(\s)data:(\s)\[[a-zA-z0-9\"\n\s,-]+]',
            new_recoveries_and_cases_str_without_spaces)
        new_recovery_values = re.sub('name:(\s)\'New Recoveries\',(\s)color:(\s)\'\#8ACA2B\',(\s)lineWidth:(\s)5,(\s)data:(\s)', '', new_recovery_values.group())
        new_recovery_values = new_recovery_values[1:-1].split(',')

        for itr, value in enumerate(new_recovery_values):
            if value == 'null':
                new_recovery_values[itr] = 0
            else:
                new_recovery_values[itr] = int(value)
        #print(f'New Recovery Values:\n{new_recovery_values}')

        # Extracting new cases values
        new_cases_values = re.search(
            r'name:(\s)\'New Cases\',(\s)color:(\s)\'\#FFC166\',(\s)lineWidth:(\s)5,(\s)data:(\s)\[[a-zA-z0-9\"\n\s,]+]',
            new_recoveries_and_cases_str_without_spaces)
        new_cases_values = re.sub(
            'name:(\s)\'New Cases\',(\s)color:(\s)\'\#FFC166\',(\s)lineWidth:(\s)5,(\s)data:(\s)', '',
            new_cases_values.group())
        new_cases_values = new_cases_values[1:-1].split(',')

        for itr, value in enumerate(new_cases_values):
            if value == 'null':
                new_cases_values[itr] = 0
            else:
                new_cases_values[itr] = int(value)
        #print(f'New Recovery Values:\n{new_cases_values}')

        # Creating dictionary for new recovery and new cases
        new_recovery = {}
        new_cases = {}
        for itr, date in enumerate(date_list):
            new_recovery[date] = new_recovery_values[itr]
            new_cases[date] = new_cases_values[itr]
        #print(f'New Recovery: {new_recovery}')
        #print(f'New Cases: {new_cases}')
        graph_stats['new_recovery'] = new_recovery
        graph_stats['new_cases'] = new_cases

    country_graphs[country_name] = graph_stats


def find_closest_active_cases_country(country_name, start_date, end_date, percent_change):
    closest_distance = 99999
    closest_country = ''
    for continent, country_list in download_website.continent_dict.items():
        for country in country_list:
            if country != country_name:
                try:
                    s_active_cases = country_graphs[country]['active_cases'][start_date]
                    e_active_cases = country_graphs[country]['active_cases'][end_date]
                    if e_active_cases != 0:
                        active_cases_percent = ((e_active_cases - s_active_cases) / e_active_cases) * 100
                        distance = abs(percent_change - abs(active_cases_percent))
                        if distance < closest_distance:
                            closest_distance = distance
                            closest_country = country
                        #print(f'{country}: {distance}')
                except KeyError:
                    dummy_val = 0

    #print(f'Closest Country: {closest_country} Closest Distance: {closest_distance}')
    return closest_country


def find_closest_daily_death_cases_country(country_name, start_date, end_date, percent_change):
    closest_distance = 99999
    closest_country = ''
    for continent, country_list in download_website.continent_dict.items():
        for country in country_list:
            if country != country_name:
                try:
                    s_active_cases = country_graphs[country]['daily_deaths'][start_date]
                    e_active_cases = country_graphs[country]['daily_deaths'][end_date]
                    if e_active_cases != 0:
                        daily_death_cases_percent = ((e_active_cases - s_active_cases) / e_active_cases) * 100
                        distance = abs(percent_change - abs(daily_death_cases_percent))
                        if distance < closest_distance:
                            closest_distance = distance
                            closest_country = country
                        #print(f'{country}: {distance}')
                except KeyError:
                    dummy_val = 0

    #print(f'Closest Country: {closest_country} Closest Distance: {closest_distance}')
    return closest_country


def find_closest_new_recovery_cases_country(country_name, start_date, end_date, percent_change):
    closest_distance = 99999
    closest_country = ''
    for continent, country_list in download_website.continent_dict.items():
        for country in country_list:
            if country != country_name:
                try:
                    s_active_cases = country_graphs[country]['new_recovery'][start_date]
                    e_active_cases = country_graphs[country]['new_recovery'][end_date]
                    if e_active_cases != 0:
                        new_recovery_cases_percent = ((e_active_cases - s_active_cases) / e_active_cases) * 100
                        distance = abs(percent_change - abs(new_recovery_cases_percent))
                        if distance < closest_distance:
                            closest_distance = distance
                            closest_country = country
                        #print(f'{country}: {distance}')
                except KeyError:
                    dummy_val = 0

    #print(f'Closest Country: {closest_country} Closest Distance: {closest_distance}')
    return closest_country


def find_closest_new_cases_country(country_name, start_date, end_date, percent_change):
    closest_distance = 99999
    closest_country = ''
    for continent, country_list in download_website.continent_dict.items():
        for country in country_list:
            if country != country_name:
                try:
                    s_active_cases = country_graphs[country]['new_cases'][start_date]
                    e_active_cases = country_graphs[country]['new_cases'][end_date]
                    if e_active_cases != 0:
                        new_cases_percent = ((e_active_cases - s_active_cases) / e_active_cases) * 100
                        distance = abs(percent_change - abs(new_cases_percent))
                        if distance < closest_distance:
                            closest_distance = distance
                            closest_country = country
                        #print(f'{country}: {distance}')
                except KeyError:
                    dummy_val = 0

    #print(f'Closest Country: {closest_country} Closest Distance: {closest_distance}')
    return closest_country


def get_yesterdays_covid_data(is_download_statistics):
    #   Download the Website
    download_website.download_website(is_download_statistics)
    index_page = common.load_data('webpages/index.html')

    #   Extracting yesterdays table data
    y_html_table = re.search(
        r'<table(\s*)id="main_table_countries_yesterday" class="[a-zA-Z0-9~`!@#$%^&\*()_\-\+=;:\'\"<,>.?\/\s\n]*>'
        r'[a-zA-Z0-9~`!@#$%^&\*()_\-\+=;:\'\"<,>.?\/\s\n]*'
        r'<div class="tab-pane " id="nav-yesterday2"', index_page)
    yesterdays_table = "\n".join(y_html_table.group().split("\n")[:-3])
    f = open('webpages/yesterday.html', 'w', encoding='utf-8')
    f.write(yesterdays_table)
    f.close()

    f = open('webpages/yesterday.html', 'r', encoding='utf-8')
    yesterdays_data = f.read()
    f.close()

    yesterdays_data = yesterdays_data.replace("\n", " ")
    yesterdays_data = re.sub(r'[\s]+', ' ', yesterdays_data)
    yesterdays_data = re.sub(r' </td>', '</td>', yesterdays_data)
    yesterdays_data = re.sub(r' style=\"[a-zA-Z0-9:; \#\%\-\!]+\"', '', yesterdays_data)
    yesterdays_data = re.sub(r'(\s)style=""', '', yesterdays_data)
    return yesterdays_data


def query_world_statistics(world_data, log_file_instance):
    print_options()
    option = input("Choose your Option:")
    try:
        selected_option = int(option)
        if selected_option in range(1, 11):
            key = get_cases_key(option)
            if world_data["world"][key] == '':
                print('DATA NOT AVAILABLE.\n')
            else:
                print(f'{world_data["world"][key]} {key} in the world yesterday.\n')
                log_file_instance.write(f'world\t{key}\t{world_data["world"][key]}\n')
        else:
            print('Invalid Input')
    except ValueError:
        print('Invalid Input')


def query_continent_statistics(continents_data, world_data, log_file_instance):
    print_continents()
    option = input("Choose your Option:")
    try:
        continent_option = int(option)
        continent_key = get_continents_key(continent_option)
        if continent_option in range(1, 7):
            print_options()
            option = input("Choose your Option:")
            selected_option = int(option)
            if selected_option in range(1, 11):
                key = get_cases_key(option)
                if continents_data[continent_key][key] == '':
                    print('DATA NOT AVAILABLE.\n')
                else:
                    world_cases = world_data["world"][key]
                    world_cases = re.sub(r'[,+]', '', world_cases)
                    continent_cases = re.sub(r'[,+]', '', continents_data[continent_key][key])
                    percent = (int(continent_cases) / int(world_cases)) * 100
                    percent = round(percent, 2)
                    print(f'{continents_data[continent_key][key]} {key} in {continent_key} yesterday.\n')
                    print(f'{continent_key} has {percent} % of {key} in the world.\n')
                    log_file_instance.write(f'{continent_key}\t{key}\t{continents_data[continent_key][key]}\n')
            else:
                print('Invalid Input')
        else:
            print('Invalid Input')
    except ValueError:
        print('Invalid Input')


def query_country_statistics(country_data, world_data, log_file_instance):
    print_countries()
    option = input("Choose your Option:")
    try:
        selected_country = int(option)
        if selected_country in range(1, 56):
            country_name = get_country_name(selected_country)
            print_options()
            print(f'\t\t11.\tTime Range')
            option = input("Choose your Option:")
            selected_option = int(option)
            if selected_option in range(1, 12):
                if selected_option == 11:
                    print(
                        '\t\t\t\tDate Format: <Month> <Day> <YEAR>\n\t\t\t\tExample: Jan 31 2021\n\t\t\t\t\t\t Mar 03 2021')
                    start_date = input("Enter Start Date:")
                    end_date = input("Enter End Date:")

                    v1_date = common.validate_date(start_date)
                    v2_date = common.validate_date(end_date)
                    if v1_date and v2_date:
                        active_cases_flag = True
                        try:
                            s_active_cases = country_graphs[country_name]['active_cases'][start_date]
                        except KeyError:
                            print(f'Active Cases data NOT found for {country_name} of {start_date}')
                            active_cases_flag = False

                        try:
                            e_active_cases = country_graphs[country_name]['active_cases'][end_date]
                        except KeyError:
                            print(f'Active Cases data NOT found for {country_name} of {end_date}')
                            active_cases_flag = False

                        if active_cases_flag:
                            active_cases_percent = ((e_active_cases - s_active_cases) / e_active_cases) * 100
                            print(f'Change in active cases: {round(active_cases_percent, 3)} %')
                            log_file_instance.write(
                                f'{country_name}\t{"range_active_cases"}\t{start_date}:{end_date}->{active_cases_percent}\n')
                            print(f'Closest Country in active cases: '
                                  f'{find_closest_active_cases_country(country_name, start_date, end_date, active_cases_percent)}\n')

                        daily_deaths_flag = True
                        try:
                            s_daily_deaths = country_graphs[country_name]['daily_deaths'][start_date]
                        except KeyError:
                            print(f'Daily Death data NOT found for {country_name} of {start_date}')
                            daily_deaths_flag = False

                        try:
                            e_daily_deaths = country_graphs[country_name]['daily_deaths'][end_date]
                        except KeyError:
                            print(f'Daily Death data NOT found for {country_name} of {end_date}')
                            daily_deaths_flag = False

                        if daily_deaths_flag:
                            daily_deaths_percent = ((e_daily_deaths - s_daily_deaths) / e_daily_deaths) * 100
                            print(f'Change in daily death cases: {round(daily_deaths_percent, 3)} %')
                            log_file_instance.write(
                                f'{country_name}\t{"range_daily_death"}\t{start_date}:{end_date}->{daily_deaths_percent}\n')
                            print(f'Closest Country in daily death cases: '
                                  f'{find_closest_daily_death_cases_country(country_name, start_date, end_date, daily_deaths_percent)}\n')

                        new_recovery_flag = True
                        try:
                            s_new_recovery = country_graphs[country_name]['new_recovery'][start_date]
                        except KeyError:
                            print(f'New Recovery data NOT found for {country_name} of {start_date}')
                            new_recovery_flag = False

                        try:
                            e_new_recovery = country_graphs[country_name]['new_recovery'][end_date]
                        except KeyError:
                            print(f'New Recovery data NOT found for {country_name} of {end_date}')
                            new_recovery_flag = False

                        if new_recovery_flag:
                            if e_new_recovery != 0:
                                new_recovery_percent = ((e_new_recovery - s_new_recovery) / e_new_recovery) * 100
                                print(f'Change in new recovery cases: {round(new_recovery_percent, 3)} %')
                                log_file_instance.write(
                                    f'{country_name}\t{"range_new_recovery"}\t{start_date}:{end_date}->{new_recovery_percent}\n')
                                print(f'Closest Country in new recovery cases: '
                                      f'{find_closest_new_recovery_cases_country(country_name, start_date, end_date, new_recovery_percent)}\n')

                        new_cases_flag = True
                        try:
                            s_new_cases = country_graphs[country_name]['new_cases'][start_date]
                        except KeyError:
                            print(f'New Cases data NOT found for {country_name} of {start_date}')
                            new_cases_flag = False

                        try:
                            e_new_cases = country_graphs[country_name]['new_cases'][end_date]
                        except KeyError:
                            print(f'New Cases data NOT found for {country_name} of {end_date}')
                            new_cases_flag = False

                        if new_cases_flag:
                            new_cases_percent = ((e_new_cases - s_new_cases) / e_new_cases) * 100
                            print(f'Change in new cases: {round(new_cases_percent, 3)} %')
                            log_file_instance.write(
                                f'{country_name}\t{"rangenew_cases"}\t{start_date}:{end_date}->{new_cases_percent}\n')
                            print(f'Closest Country in new cases: '
                                  f'{find_closest_new_cases_country(country_name, start_date, end_date, new_cases_percent)}\n')
                    else:
                        if not v1_date:
                            print(f'{start_date} not in accepted format.')
                        if not v2_date:
                            print(f'{end_date} not in accepted format.')
                else:
                    key = get_cases_key(option)
                    if country_data[country_name][key] == '':
                        print('DATA NOT AVAILABLE.\n')
                    else:
                        world_cases = world_data["world"][key]
                        world_cases = re.sub(r'[,+]', '', world_cases)
                        country_cases = country_data[country_name][key]
                        country_cases = re.sub(r'[,+]', '', country_cases)
                        percent = (int(country_cases) / int(world_cases)) * 100
                        percent = round(percent, 2)
                        print(f'{country_cases} {key} in {country_name} yesterday.')
                        log_file_instance.write(f'{country_name}\t{key}\t{country_cases}\n')
                        print(f'{world_cases} {key} in the world yesterday.')
                        print(f'{country_name} has {percent} % of {key} in the world.\n')
            else:
                print('Invalid Input')
        else:
            print('Invalid Input')
    except ValueError:
        print('Data Not Found')