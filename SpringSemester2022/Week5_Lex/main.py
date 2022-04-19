import os
import pandas as pd
import covid_country_list
import shutil
import ply.yacc as yacc
import ply.lex as lex
import re
from os import walk
from datetime import datetime
import common
import constants
import CovidStats
import covid_statistics
import numpy as np
import download_covid_wiki


world = {}
continents = {}
countries = {}
country_closest = {}
news_timeline = {}
news_response = {}
country_news_content = {}
TYPE = ""
YEAR = ""
COUNTRY = ""
COUNTRY_YEAR = 2019
tokens = [
    'TD',
    'TD_A',
    'TD_OPEN',
    'TD_CLOSE',
    'TD_STRONG',
    'NOBR',
    'TR_CONTINENT_OPEN',
    'TB_WORLD',
    'TR_CLASS',
    'TD_COUNTRY',
    'TD_COUNTRY_SPAN',
    'H3_OPEN_TAG',
    'H3_CLOSE_TAG',
    'H2_OPEN_TAG',
    'H2_CLOSE_TAG',
    'SPAN_DATE',
    'DATE',
    'SPAN_CLOSE',
    'P_OPEN',
    'P_CLOSE',
    'ON',
    'DAY',
    'MONTH',
    'PARA_START',
    'PARA_CLOSE',
    'PARA_DATA',
    'YEAR',
    'YEAR2'
]


def t_error(t):
    # print("Illegal character '%s'" % t.value[0])
    t.lexer.skip(1)


t_ignore = ' \t'
t_TD = r'<td>[\n\s\+0-9 ,.]*(N/A)*</td>'
t_TD_A = r'<td><a(\s)href="[a-zA-Z -\/]+">[\n\s\+0-9 ,.]+</a></td>'
t_TD_OPEN = r'(\s)*<td>(\s)*'
t_TD_CLOSE = r'(\s)*</td>(\s)*'
t_TD_STRONG = r'<td><strong>Total:</strong></td>'
t_NOBR = r'<nobr>[a-zA-Z \/]*</nobr>'
t_TR_CONTINENT_OPEN = r'<tr(\s)class="total_row_world(\s)row_continent"(\s)data-continent="[a-zA-Z /]+">'
t_TB_WORLD = r'<tbody(\s)class="total_row_body(\s)body_world">'
t_TR_CLASS = r'(\n\s)*<tr(\s)class="total_row">(\n\s)*'
t_TD_COUNTRY = r'<td>(<a(\s)class="mt_a"(\s)href="[a-zA-Z -\/]+")*>[éa-zA-Z -\/]+(</a>)*</td>'
t_TD_COUNTRY_SPAN = r'<td><span>[éa-zA-Z -\/]+</span></td>'
t_H3_OPEN_TAG = r'<h3>'
t_H3_CLOSE_TAG = r'</h3>(\s)*'
t_H2_OPEN_TAG = r'<h2>'
t_H2_CLOSE_TAG = r'</h2>(\s)*'
t_SPAN_DATE = r'<span(\s)class="mw-headline"(\s)id="[0-9]{1,2}(_)*(January|February|March|April|May|June|July|August|September|October|November|December)">(\s)*'
t_DATE = r'[0-9]{1,2}(\s)*(January|February|March|April|May|June|July|August|September|October|November|December)'
t_SPAN_CLOSE = r'<\/span>'
t_P_CLOSE = r'</p>'
t_ON = r'(On)'
t_DAY = r'[0-9]+'
t_MONTH = r'(January|February|March|April|May|June|July|August|September|October|November|December)'
t_PARA_START = r'(\s)*(<p_start>)(\s)*'
t_PARA_CLOSE = r'(\s)*<p_end>(\s)*'
t_PARA_DATA = r'(\s)*[Å《治理有关新型肺炎的谣言问题，这篇文章说清楚了！》广州微远基因科技ãéäía@-zA-Z0-9 ,?&;:\%\#\(\)\[\]"\'\.\-\—\–]+(\s)*'
t_YEAR = r'(January|February|March|April|May|June|July|August|September|October|November|December)*(\s)*[0-9]{4}</h3>'
t_YEAR2 = r'(January|February|March|April|May|June|July|August|September|October|November|December)*(\s)*[0-9]{4}</h2>'
t_P_OPEN = r'(On|By|From|As of|Also on|Still on)*(\s)*(the)*(\s)*[0-9]*(st)*(\s)*(January|February|March|April|May|June|July|August|September|October|November|December)[:, ]*[…Å《治理有关新型肺炎的谣言问题，这篇文章说清楚了！》广州微远基因科技ãäáéíóaúñ@-zA-Z0-9 ,$?&;:\’\‘\/\%\#\(\)\[\]"\'\.\-\—\–]+(\s)*</p>'


def p_country_news(p):
    '''
    start : YEAR
          | year2
          | data
    '''
    set_country_year(p[1])


def p_year2(p):
    '''
    year2 : YEAR2
    '''
    set_country_year(p[1])


def p_data(p):
    '''
    data : P_OPEN
    '''
    create_country_news_dict(p[1])


def p_covid_news(p):
    '''
    start : DATE PARA_DATA
    '''
    create_news_dict(p[1], p[2])


def p_start(p):
    '''
    start : continent_data
          | world_data
          | country_data
    '''


def p_continents(p):
    '''
    continent_data : TR_CONTINENT_OPEN TD TD_OPEN NOBR TD_CLOSE TD TD TD TD TD TD TD TD TD TD TD TD TD
    '''
    continent_name = covid_statistics.extract_stat(p[4])
    covid_stat = CovidStats.CovidStats(p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17])
    continents[continent_name] = covid_stat.dict_stat_data()


def p_world(p):
    '''
    world_data : TB_WORLD TR_CLASS TD TD_STRONG TD TD TD TD TD TD TD TD TD TD TD TD
    '''
    covid_stat = CovidStats.CovidStats(p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15], p[16])
    world['world'] = covid_stat.dict_stat_data()


def p_countries(p):
    '''
    country_data : TD_COUNTRY TD TD TD TD TD TD TD TD TD TD TD TD TD
                 | TD_COUNTRY_SPAN TD TD TD TD TD TD TD TD TD TD TD TD TD
                 | TD_COUNTRY TD TD TD TD TD TD TD TD TD TD TD TD TD_A
                 | TD_COUNTRY_SPAN TD TD TD TD TD TD TD TD TD TD TD TD TD_A

    '''
    covid_stats = CovidStats.CovidStats(p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13])
    countries[covid_statistics.extract_country_name(p[1])] = covid_stats.dict_stat_data()


def p_error(p):
    pass


def parse_covid_statistics(is_download_statistics):
    print('Parsing Covid Statistics Data ...')
    yesterdays_table_data = covid_statistics.get_yesterdays_covid_data(is_download_statistics)
    covid_statistics.extract_country_graphs()
    # print(country_graphs)

    lexer = lex.lex()
    lexer.input(yesterdays_table_data)

    parser = yacc.yacc()
    parser.parse(yesterdays_table_data)
    print('Parsing Covid Statistics Completed.')

    # print(world)
    # print(continents)
    # print(countries)


def create_empty_country_news():
    global country_news_content
    for country in covid_country_list.country_list:
        country = covid_country_list.country_list_to_file_country[country]
        country_news_content[country] = {}
        for year in range(2019, 2023):
            country_news_content[country][year] = {}
            for month in range(1, 13):
                country_news_content[country][year][month] = {}
                for day in range(1, 32):
                    country_news_content[country][year][month][day] = None


def create_country_news_dict(news_string):
    global country_news_content
    if len(news_string) > 50:
        day, month = extract_day_month(news_string)
        if day != None and month != None:
            news = re.sub(r'</p>', '', news_string)
            #print(f'{COUNTRY}')
            #print(f'{COUNTRY_YEAR}')
            #print(f'{month}')
            #print(f'{day}')
            #print(f'{news_string}')
            if country_news_content[COUNTRY][COUNTRY_YEAR][month][day] != None:
                country_news_content[COUNTRY][COUNTRY_YEAR][month][day] += str(news)
            else:
                country_news_content[COUNTRY][COUNTRY_YEAR][month][day] = str(news)


def create_empty_news():
    global news_timeline
    global news_response
    for year in range(2019, 2023):
        news_timeline[year] = {}
        news_response[year] = {}
        for month in range(1, 13):
            news_timeline[year][month] = {}
            news_response[year][month] = {}
            for day in range(1, 32):
                news_timeline[year][month][day] = None
                news_response[year][month][day] = None


def create_news_dict(date, news_content):
    global YEAR
    global TYPE
    #global month_to_index
    global news_timeline

    splitted_date = date.split(' ')
    day = splitted_date[0]
    month = splitted_date[1]
    month_index = common.month_to_index[month]
    if TYPE == "timeline":
        news_timeline[int(YEAR)][month_index][int(day)] = news_content
    elif TYPE == "response":
        if news_response[int(YEAR)][month_index][int(day)] != None :
            news_response[int(YEAR)][month_index][int(day)] += news_content
        else:
            news_response[int(YEAR)][month_index][int(day)] = news_content


def preprocess_files(dir, file):

    filename = dir + "/" + str(file)
    processed_filename = dir + "/" + "processed" + "/" + str(file)

    file = open(filename, 'r')
    file_data = file.read()
    file.close()

    file_data = file_data.replace("\n", " ")
    tags_to_remove = [common.anchor_open_tag, common.anchor_close_tag, common.style_with_content, common.style_open_tag,
                      common.style_close_tag, common.span_edit_section, common.span_title, common.span_close,
                      common.sup_open, common.sup_close, common.sup_tag_t1, common.sup_tag_t2, common.sup_tag_t3,
                      common.bold_open, common.bold_close, common.italics_open, common.italics_close,
                      common.sup_noprint_tag, common.span_better_source, common.edit, common.h3_close,
                      common.div_note, common.p_open, common.p_close, common.ul_open, common.ul_close,
                      common.li_open, common.li_class, common.li_close, common.dl_open, common.dl_close,
                      common.dd_open, common.dd_close, common.div_t0, common.div_t1, common.div_t2, common.div_t3,
                      common.img, common.div_close, common.span_class_id, common.h4_open, common.h4_close,
                      common.sup_tag_t4, common.span_class_style, common.image_box_data,
                      common.div_t4, common.gib_1
                      ]

    for tags in tags_to_remove:
        file_data = re.sub(tags, '', file_data)

    # Check in file is 2019
    if filename.__contains__("2019"):
        file_data = re.sub(r'</h2>', '', file_data)

    file_ptr = open(processed_filename, 'w')
    file_ptr.write(file_data)
    file_ptr.close()


def get_files(path):
    filenames = []
    for (dir_path, dir_names, files) in walk(path):
        filenames.extend(files)
        break

    for file in filenames:
        if not str(file).__contains__(".html"):
            filenames.remove(file)

    return filenames


def parse_covid_news_files(filenames, file_dir, type):#
    global YEAR
    global TYPE
    for file in filenames:
        filename = file_dir + "/" + "processed" + "/" + str(file)

        year = re.search(r'[0-9]{4}', filename)
        YEAR = year.group()
        TYPE = type
        f = open(filename, 'r')
        data = f.read()
        f.close()

        print(f'Parsing {filename}')
        covid_news_lex = lex.lex()
        covid_news_lex.input(data)

        covid_news = yacc.yacc()
        covid_news.parse(data)


def get_news_content(type, start_date, end_date):
    split_start_date = start_date.split('-')
    s_d = int(split_start_date[0])
    s_m = int(split_start_date[1])
    s_y = int(split_start_date[2])

    split_end_date = end_date.split('-')
    e_d = int(split_end_date[0])
    e_m = int(split_end_date[1])
    e_y = int(split_end_date[2])
    news = ""
    for y in range(s_y, e_y + 1):
        if s_y == e_y:
            for m in range(s_m, e_m + 1):
                if s_m == e_m and s_y == e_y:
                    for d in range(s_d, e_d + 1):
                        if type == constants.TIMELINE:
                            if news_timeline[y][m][d] != None:
                                date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                                news = "\n" + date + news_timeline[y][m][d] + news
                        elif type == constants.RESPONSE:
                            if news_response[y][m][d] != None:
                                date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                                news = "\n" + date + news_response[y][m][d] + news
                else:
                    for d in range(s_d, 32):
                        if type == constants.TIMELINE:
                            if news_timeline[y][m][d] != None:
                                date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                                news = "\n" + date + news_timeline[y][m][d] + news
                        elif type == constants.RESPONSE:
                            if news_response[y][m][d] != None:
                                date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                                news = "\n" + date + news_response[y][m][d] + news
                    for d in range(1, e_d + 1):
                        if type == constants.TIMELINE:
                            if news_timeline[y][m][d] != None:
                                date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                                news = "\n" + date + news_timeline[y][m][d] + news
                        elif type == constants.RESPONSE:
                            if news_response[y][m][d] != None:
                                date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                                news = "\n" + date + news_response[y][m][d] + news
        else:
            if y != e_y:
                itr = 0
                for m in range(s_m, 13):
                    if itr == 0:
                        start = s_d
                    else:
                        start = 1
                    for d in range(start, 32):
                        if type == constants.TIMELINE:
                            if news_timeline[y][m][d] != None:
                                date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                                news = "\n" + date + news_timeline[y][m][d] + news
                        elif type == constants.RESPONSE:
                            if news_response[y][m][d] != None:
                                date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                                news = "\n" + date + news_response[y][m][d] + news
                    itr += 1
            else:
                for m in range(1, e_m + 1):
                    for d in range(1, e_d + 1):
                        if type == constants.TIMELINE:
                            if news_timeline[y][m][d] != None:
                                date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                                news = "\n" + date + news_timeline[y][m][d] + news
                        elif type == constants.RESPONSE:
                            if news_response[y][m][d] != None:
                                date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                                news = "\n" + date + news_response[y][m][d] + news

    return news


def split_date(date):
    split_date = date.split("-")
    return int(split_date[0]), int(split_date[1]), int(split_date[2])


def non_overlapping_date_range(start_date1, end_date1, start_date2, end_date2):

    s1_d, s1_m, s1_y = split_date(start_date1)
    e1_d, e1_m, e1_y = split_date(end_date1)
    s2_d, s2_m, s2_y = split_date(start_date2)
    e2_d, e2_m, e2_y = split_date(end_date2)

    s1 = datetime(s1_y, s1_m, s1_d)
    e1 = datetime(e1_y, e1_m, e1_d)
    s2 = datetime(s2_y, s2_m, s2_d)
    e2 = datetime(e2_y, e2_m, e2_d)

    if s1 <= e2 and s2 <= e1:
        return False
    return True


def process_country_pages(page_content):
    page_content = page_content.replace("\n", " ")
    tags_to_remove = [common.style_with_content, common.style_open_tag, common.style_close_tag,
                      common.anchor_open_tag, common.anchor_close_tag, common.sup_open, common.sup_tag_t0,
                      common.sup_tag_t1, common.sup_tag_t2, common.sup_tag_t3, common.sup_tag_t4,
                      common.sup_tag_t5, common.sup_tag_t6, common.sup_tag_t7, common.sup_tag_t8, common.sup_close, common.italics_open,
                      common.italics_close, common.span_title, common.span_close, common.span_class_nowrap, common.color_code, common.span_class_id,
                      common.span_class, common.edit1, common.bold_close, common.bold_open, common.ul_open,
                      common.ul_close, common.span_class, common.sup, common.line_break, common.anchor_tag1]
    for tags in tags_to_remove:
        page_content = re.sub(tags, '', page_content)

    page_content = re.sub('<li>', '<p>', page_content)
    page_content = re.sub('</li>', '</p>', page_content)
    page_content = re.sub('<h4>', '', page_content)
    page_content = re.sub('</h4> <p>', ' ', page_content)
    page_content = re.sub('<h2>', '<test> <h2>', page_content)
    page_content = re.sub('</h2>', '</h2> </test>', page_content)
    page_content = re.sub('<h3>', '<test> <h3>', page_content)
    page_content = re.sub('</h3>', '</h3> </test>', page_content)
    page_content = re.sub('<p></p>', '<p> </p>', page_content)
    page_content = re.sub('January <p>', 'January ', page_content)
    page_content = re.sub('February <p>', 'February ', page_content)
    page_content = re.sub('March <p>', 'March ', page_content)
    page_content = re.sub('April <p>', 'April ', page_content)
    page_content = re.sub('May <p>', 'May ', page_content)
    page_content = re.sub('June <p>', 'June ', page_content)
    page_content = re.sub('July <p>', 'July ', page_content)
    page_content = re.sub('August <p>', 'August ', page_content)
    page_content = re.sub('September <p>', 'September ', page_content)
    page_content = re.sub('October <p>', 'October ', page_content)
    page_content = re.sub('November <p>', 'November ', page_content)
    page_content = re.sub('December <p>', 'December ', page_content)
    page_content = re.sub('January</h3> </test> <p>', 'January ', page_content)
    page_content = re.sub('February</h3> </test> <p>', 'February ', page_content)
    page_content = re.sub('March</h3> </test> <p>', 'March ', page_content)
    page_content = re.sub('April</h3> </test> <p>', 'April ', page_content)
    page_content = re.sub('May</h3> </test> <p>', 'May ', page_content)
    page_content = re.sub('June</h3> </test> <p>', 'June ', page_content)
    page_content = re.sub('July</h3> </test> <p>', 'July ', page_content)
    page_content = re.sub('August</h3> </test> <p>', 'August ', page_content)
    page_content = re.sub('September</h3> </test> <p>', 'September ', page_content)
    page_content = re.sub('October</h3> </test> <p>', 'October ', page_content)
    page_content = re.sub('November</h3> </test> <p>', 'November ', page_content)
    page_content = re.sub('December</h3> </test> <p>', 'December ', page_content)
    page_content = re.sub(r'</p>(\s)</p>', '', page_content)
    return page_content


def extract_country(string_with_country):
    extracted_country = re.search(r'country\/[a-zA-Z_]*', string_with_country)
    if extracted_country != None:
        country = extracted_country.group()
        country = re.sub(r'country/', '', country)
        return country


def extract_year(string_with_year):
    extracted_year = re.search(r'[0-9]{4}', string_with_year)
    if extracted_year != None:
        return int(extracted_year.group())


def extract_day_month(news_string):
    day = None
    month = None
    day_month = re.search(r'[0-9]{1,2}(\s)(January|February|March|April|May|June|July|August|September|October|November|December)', news_string)
    if day_month == None:
        day_month = re.search(
            r'(January|February|March|April|May|June|July|August|September|October|November|December)(\s)[0-9]{1,2}',
            news_string)
        if day_month != None:
            day_month_string = str(day_month.group())
            month, day = day_month_string.split(' ')
    else:
        day_month_string = str(day_month.group())
        day, month = day_month_string.split(' ')

    if day != None and month != None:
        return int(day), common.month_to_index[month]
    return None, None


def set_country_year(year_string):
    global COUNTRY_YEAR
    if year_string != None:
        year = extract_year(year_string)
        COUNTRY_YEAR = year
        #print(f'YEAR: {COUNTRY_YEAR}')


def process_country_news():
    global COUNTRY_YEAR
    global  COUNTRY
    create_empty_country_news()
    country_news_dir = "covid_wiki/country/"
    country_dirs = []
    processed_files = []

    for (root, dirs, files) in os.walk(country_news_dir, topdown=True):
        for file in files:
            str_file = str(file)
            if str_file.__contains__(".html") and not str_file.__contains__("processed"):
                file_dir = root + "/" + file
                country_dirs.append(file_dir)

    for country_page in country_dirs:
        str_file_dir = str(country_page)
        splitted_file_text = str_file_dir.split("/")
        processed_file_dir = country_news_dir + splitted_file_text[2] + "/processed/" + splitted_file_text[3]

        if not processed_file_dir.__contains__("/processed/processed"):
            processed_files.append(processed_file_dir)

        file = open(country_page, 'r')
        file_data = file.read()
        file.close()

        print(f'Processing {country_page}')
        file = open(processed_file_dir, 'w')
        file.write(process_country_pages(file_data))
        file.close()

    for file in processed_files:
        year = extract_year(file)
        country = extract_country(file)

        if year:
            COUNTRY_YEAR = year
        if country:
            COUNTRY = covid_country_list.country_list_to_file_country[country]

        processed_file = open(file, 'r')
        data = processed_file.read()
        processed_file.close()

        print(f'Parsing {file}')
        country_news_lex = lex.lex()
        country_news_lex.input(data)

        country_news = yacc.yacc()
        country_news.parse(data)


def find_news_time_range(country):
    global country_news_content
    start_month = 0
    start_year = 0
    end_month = 0
    end_year = 0
    brk = 0
    for year in range(2019, 2023):
        for month in range(1, 13):
            for day in range(1, 32):
                if country_news_content[country][year][month][day] != None:
                    #print(country_news_content[country][year][month][day])
                    start_month = month
                    start_year = year
                    brk = 1
                    break
            if brk == 1:
                break
        if brk == 1:
            break

    #print(dict_country)
    #print(f'Day: {day} Month: {start_month} Year {start_year}')
    #print('Prev News:')

    brk = 0
    for year in reversed(range(2018, 2023)):
        for month in reversed(range(1, 13)):
            for day in reversed(range(1, 32)):
                if country_news_content[country][year][month][day] != None:
                    #print(country_news_content[country][year][month][day])
                    end_month = month
                    end_year = year
                    brk = 1
                    break
            if brk == 1:
                break
        if brk == 1:
            break

    #print(dict_country)
    #print(f'Day: {day} Month: {end_month} Year {end_year}')
    #print('Prev News:')
    if country == "Bangladesh":
        start_year = 2020
        end_year = 2020
    if country == "Canada":
        end_month = 4
    if country == "Ghana":
        end_month = 2
        end_year = 2022
    if country == "the_Philippines":
        end_month = 2
    if country == "Spain":
        end_month = 6

    print(f'News Date Range for {country}: {common.index_to_month[start_month]},{start_year}-{common.index_to_month[end_month]},{end_year}')


def get_country_news_content(country, start_date, end_date):
    global country_news_content
    split_start_date = start_date.split('-')
    s_d = int(split_start_date[0])
    s_m = int(split_start_date[1])
    s_y = int(split_start_date[2])

    split_end_date = end_date.split('-')
    e_d = int(split_end_date[0])
    e_m = int(split_end_date[1])
    e_y = int(split_end_date[2])
    news = ""
    for y in range(s_y, e_y + 1):
        if s_y == e_y:
            for m in range(s_m, e_m + 1):
                if s_m == e_m and s_y == e_y:
                    for d in range(s_d, e_d + 1):
                        if country_news_content[country][y][m][d] != None:
                            date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                            news = "\n" + date + country_news_content[country][y][m][d] + news
                else:
                    for d in range(s_d, 32):
                        if country_news_content[country][y][m][d] != None:
                            date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                            news = "\n" + date + country_news_content[country][y][m][d] + news
                    for d in range(1, e_d + 1):
                        if country_news_content[country][y][m][d] != None:
                            date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                            news = "\n" + date + country_news_content[country][y][m][d] + news
        else:
            if y != e_y:
                itr = 0
                for m in range(s_m, 13):
                    if itr == 0:
                        start = s_d
                    else:
                        start = 1
                    for d in range(start, 32):
                        if country_news_content[country][y][m][d] != None:
                            date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                            news = "\n" + date + country_news_content[country][y][m][d] + news
                    itr += 1
            else:
                for m in range(1, e_m + 1):
                    for d in range(1, e_d + 1):
                        if country_news_content[country][y][m][d] != None:
                            date = common.index_to_month[m] + " " + str(d) + "," + str(y) + "\n"
                            news = "\n" + date + country_news_content[country][y][m][d] + news
    return news


def calculate_jaccard_similarity(source_country, target_country):
    source = set(source_country.lower().split())
    target = set(target_country.lower().split())

    intersection = source.intersection(target)

    union = source.union(target)

    return 1.0 - (float(len(intersection)) / len(union))


def get_target_countries(source_country):
    target_countries = []
    all_countries = covid_country_list.file_country_to_country_list.keys()
    for country in all_countries:
        if country != source_country:
            target_countries.append(country)
    return target_countries


def display_top_k(jaccard_distances):
    sorted_index = np.argsort(jaccard_distances)
    #print(sorted_index)
    print('Top 3:')
    k = 0
    for itr in sorted_index:
        if k < 3:
            print(covid_country_list.country_list[itr])
        k += 1


def find_top3_with_jaccard(source_country, start_date, end_date):
    jaccard_distances = []

    source_country_news = get_country_news_content(source_country, start_date, end_date)
    source_country_news_without_sw = common.remove_stop_words(source_country_news)
    source = ' '.join(source_country_news_without_sw)

    target_countries = get_target_countries(source_country)
    #print(f'Target Countries: {target_countries}')

    for country in target_countries:
        target_country_news = get_country_news_content(country, start_date, end_date)
        target_country_news_without_sw = common.remove_stop_words(target_country_news)
        target = ' '.join(target_country_news_without_sw)
        distance = calculate_jaccard_similarity(source, target)
        jaccard_distances.append(distance)

    display_top_k(jaccard_distances)


def find_top3_with_jaccard_for_covidwords(source_country, start_date, end_date):
    jaccard_distances = []

    source_country_news = get_country_news_content(source_country, start_date, end_date)
    source_country_news_covidwords = common.remove_non_covid_words(source_country_news)
    source = ' '.join(source_country_news_covidwords)

    target_countries = get_target_countries(source_country)
    #print(f'Target Countries: {target_countries}')

    for country in target_countries:
        target_country_news = get_country_news_content(country, start_date, end_date)
        target_country_news_covidwords = common.remove_stop_words(target_country_news)
        target = ' '.join(target_country_news_covidwords)
        distance = calculate_jaccard_similarity(source, target)
        jaccard_distances.append(distance)

    display_top_k(jaccard_distances)


if __name__ == '__main__':

    common.load_covid_words()
    is_download_statistics = 0
    #download_latest_covid_statistics = input("Do you want to download latest wiki covid statistics(Y/N):")
    #if download_latest_covid_statistics == "Y" or download_latest_covid_statistics == "y":
    #    is_download_statistics = 1
    #else:
    #   is_download_statistics = 0
    is_download_statistics = 1

    #download_latest_wiki_data = input("Do you want to download latest wiki covid news(Y/N):")
    #if download_latest_wiki_data == "Y" or download_latest_wiki_data == "y":
    #    download_covid_wiki.download_website()
    download_covid_wiki.download_website()

    parse_covid_statistics(is_download_statistics)
    create_empty_news()
    timeline_dir = "covid_wiki/timeline"
    response_dir = "covid_wiki/responses"
    timeline_files = get_files(timeline_dir)
    responses_files = get_files(response_dir)

    for file in timeline_files:
        preprocess_files(timeline_dir, file)

    for file in responses_files:
        preprocess_files(response_dir, file)
    
    parse_covid_news_files(timeline_files, timeline_dir, "timeline")
    parse_covid_news_files(responses_files, response_dir, "response")

    process_country_news()

    date = datetime.now().strftime("%Y%m%d%I%M%S")
    log_filename = "logs/log_" + date + ".txt";
    log_file = open(log_filename, "a")
    log_file.write(f'world/continent/country\tfield\tresult\n')
    print("Welcome to Worldometers!")

    while True:
        screen_columns = shutil.get_terminal_size().columns
        print("\n1.\tWorld Covid Statistics\n2.\tContinent Covid Statistics\n"
              "3.\tCountry Covid Statistics\n4.\tWorldwide Wikipedia News\n5.\tNon-Overlapping Time Range \n6.\tCountry News \n7.\tPress E to Exit!!!\n")
        option = input("Choose your Option:")
        if option == 'E' or option == 'e':
            exit(0)

        try:
            title = ""
            selected_option = int(option)
            if selected_option in range(1, 7):
                if selected_option == 1:
                    covid_statistics.query_world_statistics(world, log_file)
                elif selected_option == 2:
                    covid_statistics.query_continent_statistics(continents, world, log_file)
                elif selected_option == 3:
                    covid_statistics.query_country_statistics(countries, world, log_file)
                elif selected_option == 4:
                    print('1.\tWorld Wide News\n2.\tWorlwide Response')
                    option = int(input('Choose your option:'))
                    start_date = input('Enter the start date[DD-MM-YYYY]:')
                    end_date = input('Enter the end date[DD-MM-YYYY]:')

                    if common.validate_news_date(start_date) and common.validate_news_date(end_date):
                        if common.validate_date_range(start_date, end_date):
                            if option == 1:
                                timeline_news_content = get_news_content(constants.TIMELINE, start_date, end_date)
                                print(timeline_news_content)
                                title = "Wordcloud for timeline news from " + start_date + " to " + end_date
                                common.show_word_cloud(timeline_news_content, title)
                            elif option == 2:
                                response_news_content = get_news_content(constants.RESPONSE, start_date, end_date)
                                print(response_news_content)
                                title = "Wordcloud for response news from " + start_date + " to " + end_date
                                common.show_word_cloud(response_news_content, title)
                            else:
                                print("Invalid Input")
                        else:
                            print('Date out of range')
                    else:
                        print('Incorrect Date Format.')
                elif selected_option == 5:
                    start_date1 = input("Enter start date of first time range[DD-MM-YYYY]:")
                    end_date1 = input("Enter the end date of first time range[DD-MM-YYYY]:")
                    start_date2 = input("Enter start date of second time range[DD-MM-YYYY]:")
                    end_date2 = input("Enter the end date of second time range[DD-MM-YYYY]:")

                    if common.validate_news_date(start_date1) and common.validate_news_date(end_date1) and common.validate_news_date(start_date2) and common.validate_news_date(end_date2):
                        if common.validate_date_range(start_date1, end_date1) and common.validate_date_range(start_date2, end_date2):
                            if non_overlapping_date_range(start_date1, end_date1, start_date2, end_date2):
                                range1_news_content = get_news_content(constants.TIMELINE, start_date1, end_date1)
                                range1_news_content_without_sw = common.remove_stop_words(range1_news_content)
                                covid1_words = common.remove_non_covid_words(range1_news_content_without_sw)

                                range2_news_content = get_news_content(constants.TIMELINE, start_date2, end_date2)
                                range2_news_content_without_sw = common.remove_stop_words(range2_news_content)
                                covid2_words = common.remove_non_covid_words(range2_news_content_without_sw)

                                news_content_without_sw = range1_news_content_without_sw + range2_news_content_without_sw
                                news_content_covid_words = covid1_words + covid2_words
                                print('\n1.\tWordcloud for common words and covid words\n2.\tPercentage of covid related '
                                      'words in common words\n3.\tTop 20 Common words and Covid Words')
                                option = int(input("\nEnter your option:"))
                                if option == 1:
                                    title = "Common Words(without stopwords) from " + start_date1 + " to " + end_date1 + " and " + start_date2 + " to " + end_date2
                                    common.show_word_cloud(' '.join(news_content_without_sw), title)

                                    title = "Covid Words(without stopwords) from " + start_date1 + " to " + end_date1 + " and " + start_date2 + " to " + end_date2
                                    common.show_word_cloud(' '.join(news_content_covid_words), title)
                                    '''
                                    title = "Common Words(without stopwords) from " + start_date1 + " to " + end_date1
                                    common.show_word_cloud(' '.join(range1_news_content_without_sw), title)

                                    title = "Covid Words(without stopwords) from " + start_date1 + " to " + end_date1
                                    common.show_word_cloud(' '.join(covid1_words), title)

                                    title = "Common Words(without stopwords) from " + start_date2 + " to " + end_date2
                                    common.show_word_cloud(' '.join(range2_news_content_without_sw), title)

                                    title = "Covid Words(without stopwords) from " + start_date1 + " to " + end_date1
                                    common.show_word_cloud(' '.join(covid2_words), title)
                                    '''
                                elif option == 2:
                                    #print("percentage selected")
                                    len_common_words1 = len(range1_news_content_without_sw)
                                    len_covid_words1 = len(covid1_words)

                                    len_common_words2 = len(range2_news_content_without_sw)
                                    len_covid_words2 = len(covid2_words)

                                    commons = len_common_words1 + len_common_words2
                                    covid_words = len_covid_words1 + len_covid_words2

                                    #if len_common_words1 > 0 and len_common_words2 > 0:
                                    #    covid_words_percent_t1 = (len_covid_words1 / len_common_words1) * 100
                                    #    covid_words_percent_t2 = (len_covid_words2 / len_common_words2) * 100
                                    if commons > 0:
                                        covid_word_percent = (covid_words / commons) * 100
                                    print(
                                        f'Percentage of Covid words: {round(covid_word_percent, 2)} %')
                                    #print(f'Percentage of Covid words in first timerange: {round(covid_words_percent_t1, 2)} %')
                                    #print(f'Percentage of Covid words in second timerange: {round(covid_words_percent_t2, 2)} %')
                                elif option == 3:
                                    r1_common_counts = pd.value_counts(np.array(range1_news_content_without_sw))
                                    r1_covid_counts = pd.value_counts(np.array(covid1_words))
                                    r2_common_counts = pd.value_counts(np.array(range2_news_content_without_sw))
                                    r2_covid_counts = pd.value_counts(np.array(covid2_words))

                                    common_counts = r1_common_counts + r2_common_counts
                                    covid_counts = r1_covid_counts + r2_covid_counts

                                    print("\nTop 20 common words:")
                                    print(common_counts.sort_values(ascending=False).head(20))

                                    print("\nTop 20 covid words:")
                                    print(covid_counts.sort_values(ascending=False).head(20))

                                    #print("\nTop 20 common words of first time-range:")
                                    #print(r1_common_counts.sort_values(ascending=False).head(20))

                                    #print("\nTop 20 covid words of first time-range:")
                                    #print(r1_covid_counts.sort_values(ascending=False).head(20))

                                    #print("\nTop 20 common words of second time-range:")
                                    #print(r2_common_counts.sort_values(ascending=False).head(20))

                                    #print("\nTop 20 covid words of second time-range:")
                                    #print(r2_covid_counts.sort_values(ascending=False).head(20))
                                else:
                                    print('Incorrect Option selected.')
                            else:
                                print("Date range is overlapping.")
                        else:
                            print('Date out of range')
                    else:
                        print('Incorrect Date Format.')
                elif selected_option == 6:
                    n_country = len(covid_country_list.country_list)
                    for itr, country in enumerate(covid_country_list.country_list):
                        print(f'\t{itr + 1}. {country}')
                    option = input(f'\nEnter your option[{1}-{n_country}]:')
                    if int(option) in range(1, n_country + 1):
                        print("1.\tFind Date Range of News\n2.\tFind news for a given date range\n3.\tTop 3 country for with Jaccard Similarity\n4.\tTop 3 country(covid words) with Jaccard Similarity")
                        dr_option = input("Enter your option:")
                        selected_dr_option = int(dr_option)
                        selected_country = covid_country_list.country_list[int(option) - 1]
                        dict_country = covid_country_list.country_list_to_file_country[selected_country]
                        if selected_dr_option in range(1, 5):
                            if selected_dr_option == 1:
                                find_news_time_range(dict_country)
                            elif selected_dr_option == 2:
                                start_date = input("Enter start date[DD-MM-YYYY]:")
                                end_date = input("Enter the end date[DD-MM-YYYY]:")
                                if common.validate_news_date(start_date) and common.validate_news_date(end_date):
                                    news = get_country_news_content(dict_country, start_date, end_date)
                                    print(news)
                                    news_without_sw = common.remove_stop_words(news)
                                    title = "Word Cloud for news content of " + dict_country + " from " + str(start_date) + " to " + str(end_date)
                                    common.show_word_cloud(' '.join(news_without_sw), title)
                                else:
                                    print('Incorrect Date Format.')
                            elif selected_dr_option == 3:
                                start_date = input("Enter start date for Jaccard Similarity(common)[DD-MM-YYYY]:")
                                end_date = input("Enter the end date for Jaccard Similarity(common)[DD-MM-YYYY]:")
                                if common.validate_news_date(start_date) and common.validate_news_date(end_date):
                                    find_top3_with_jaccard(dict_country, start_date, end_date)
                                else:
                                    print('Incorrect Date Format.')
                            elif selected_dr_option == 4:
                                start_date = input("Enter start date for Jaccard Similarity(covid-words)[DD-MM-YYYY]:")
                                end_date = input("Enter the end date for Jaccard Similarity(covid-words)[DD-MM-YYYY]:")
                                if common.validate_news_date(start_date) and common.validate_news_date(end_date):
                                    find_top3_with_jaccard_for_covidwords(dict_country, start_date, end_date)
                                else:
                                    print('Incorrect Date Format.')
                        else:
                            print(f'Invalid option selected.')
                    else:
                        print('Invalid option selected.')
            else:
                print('Invalid Input')
        except ValueError:
            print('Invalid Input')
    log_file.close()
