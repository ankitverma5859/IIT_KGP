import re
import matplotlib.pyplot as plt
from wordcloud import WordCloud
from nltk.corpus import stopwords
from nltk.tokenize import word_tokenize

month_to_index = {
    "January": 1,
    "February": 2,
    "March": 3,
    "April": 4,
    "May": 5,
    "June": 6,
    "July": 7,
    "August": 8,
    "September": 9,
    "October": 10,
    "November": 11,
    "December": 12,
}

index_to_month = {
        1: "January",
        2: "February",
        3: "March",
        4: "April",
        5: "May",
        6: "June",
        7: "July",
        8: "August",
        9: "September",
        10: "October",
        11: "November",
        12: "December",
    }

anchor_open_tag = r'<a[ a-zA-Z0-9№=,\+:–‑;&?\@\$\(\)\[\]\#"_\-\#\/\.%\r\nùéãíÅçÖóàöð]*>'
anchor_tag1 = r'<a href="/wiki/Hospital_Universitario_Nuestra_Se%C3%B1ora_de_Candelaria" title="Hospital Universitario Nuestra Señora de Candelaria">'
anchor_close_tag = r'</a>'
style_with_content = r'<style[ a-zA-Z0-9=,:–‑;&?\(\)\[\]\#"_\-\#\/\.%\r\nùéãíÅçÖóàöð]*>[a-zA-Z0-9{}%:;\-\.\s\+]*<\/style>'
style_open_tag = r'<style[ a-zA-Z0-9=,:–‑;&?\(\)\[\]\#"_\-\#\/\.%\r\nùéãíÅçÖóàöð]*>'
style_close_tag = r'</style>'
span_edit_section = r'<span class="[a-z -]*">[\[\]]*(\s)*(edit)*(\s)*'
span_title = r'<span title="[a-zA-Z0-9 \(\)\.]*">'
span_close = r'</span>'
span_class_style = r'<span class="legend-color" style="[a-zA-Z0-9 :;\#\-]*">'
sup_open = r'<sup>'
sup_close = r'</sup>'
sup_tag_t0 = r'<sup id="cite_ref-death.[0-9_-]*"(\s)class="reference">[ 0-9&#]*;'
sup_tag_t1 = r'<sup id="cite_ref-[a-zA-Z0-9_-]*"(\s)class="reference">[ 0-9&#;]*'
sup_tag_t2 = r'<sup(\s)*id="cite_ref-:[a-zA-Z0-9_-]*"(\s)*class="reference">'
sup_tag_t3 = r'<sup(\s)*class="[a-zA-Z -]*"(\s)(style="[a-z:;-]*")*>'
sup_tag_t4 = r'<sup id="cite_ref-web.archive.org_[0-9 -]*" class="reference">'
sup_tag_t5 = r'<sup id="cite_ref-corona.gov.bd_[0-9 -]*" class="reference">'
sup_tag_t6 = r'<sup id="cite_ref-geo.tv_[0-9 -]*" class="reference">'
sup_tag_t7 = r'<sup id="cite_ref-abc-econ-stim-nsw-mar[0-9 _ \-]*" class="reference">'
sup_tag_t8 = r'<sup id="cite_ref-abc-vic-nsw/vic-border_re-close_[0-9 _ \-]*" class="reference">'
sup = '<sup id="[a-zA-Z0-9№ \%\#\.\+\=\@\$\&;_-]*" class="[a-z]*">'
sup_noprint_tag = r'<sup(\s)*class="[a-zA-Z -]*"(\s)(style="white-space:nowrap;")*>[ 0-9&#;]*'
span_better_source = r'<span title="[a-zA-Z0-9 \.\(\)]*">[a-zA-Z0-9 &;\#]*<\/span>'
bold_open = r'<b>'
bold_close = r'</b>'
italics_open = r'<i>'
italics_close = r'</i>'
edit = r'edit'
edit1 = r'\[edit\]'
color_code = r'&[0-9#]*;'
span_class_nowrap = r'<span class="nowrap">'
span_class = r'<span class="[a-zA-Z -]*">'
span_class_id = r'<span class="mw-headline" id="[a-zA-Z0-9_]*">'
div_note = r'<div role="note" class="hatnote navigation-not-searchable">[a-zA-Z0-9\s:\-]*<\/div>'
p_open = r'<p>'
p_close = r'</p>'
p_openclose = r'</p><p>'
ul_open = r'<ul>'
ul_close = r'</ul>'
li_open = r'<li>'
li_class = r'<li class="[a-zA-Z \-]*">'
li_close = r'</li>'
dl_open = r'<dl>'
dl_close = r'</dl>'
dd_open = r'<dd>'
dd_close = r'</dd>'
line_break = r'<br(\s)*/>'
div_t0 = r'<div class="thumb tright">'
div_t1 = r'<div class="thumbinner" style="[a-z0-9 :;]*">'
div_t2 = r'<div class="thumbcaption">'
div_t3 = r'<div class="magnify">'
div_t4 = r'<div class="legend">'
img = r'<img alt="" src="[a-zA-Z0-9_\-\/\.]*" decoding="async" width="[0-9]*" height="[0-9]*" class="thumbimage" srcset="[a-zA-Z0-9 _,\-\/\.]*" data-file-width="[0-9]*" data-file-height="[0-9]*" \/>'
div_close = r'</div>'
h3_close = r'</h3>'
h4_open = r'<h4>'
h4_close = r'</h4>'
image_box_data = r'<div class="legend">[ a-zA-Z0-9;:,&\#]*<link rel="mw-deduplicated-inline-style" href="[a-zA-Z0-9 :\-]*"\/>'
gib_1 = r'Map of the WHO\'s regional offices and their respective operating regions. &#160;&#160;Americas; HQ: Washington, D.C.,'

covid_words = []
stop_words = set(stopwords.words('english'))
corpus_specific_sw = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September',
                      'October', 'November', 'December', '2019', '2020', '2021', '2022', '.', ',', '(', ')', ';',
                      '"', '\'']
stop_words.update(corpus_specific_sw)


def load_data(file_location):
    f = open(file_location, 'r')
    contents = f.read()
    f.close()
    return contents

'''
    Functions related to words
'''


def remove_stop_words(content):
    global stop_words
    words = word_tokenize(content)
    tokens_without_sw = [word for word in words if not word.lower() in stop_words]
    return tokens_without_sw


def remove_non_covid_words(content):
    global covid_words
    word_list = []
    for word in content:
        if word in covid_words:
            word_list.append(word)
    #print(word_list)
    return word_list


def load_covid_words():
    global covid_words
    file = open('resources/covid_word_dictionary.txt', 'r')
    file_content = file.read()
    file.close()

    covid_words = word_tokenize(file_content)


def show_word_cloud(content, title):
    word_cloud = WordCloud().generate(content)
    plt.imshow(word_cloud)
    plt.axis("off")
    plt.title(title)
    plt.show()


'''
    Date related functions
'''


def validate_date(date):
    if re.match(r'(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sept|Oct|Nov|Dec|) [0-9]{2} [0-9]{4}', date):
        return True
    return False


def validate_news_date(date):
    if re.match(r'[0-9]{1,2}-[0-9]{1,2}-[0-9]{4}', date):
        return True
    return False


def validate_date_range(start_date, end_date):
    '''
    Case 1: Start Date and End Date are both below the start date
    Case 2: Start Date and End Date are both after the end date
    '''
    split_start_date = start_date.split('-')
    s_d = int(split_start_date[0])
    s_m = int(split_start_date[1])
    s_y = int(split_start_date[2])

    split_end_date = end_date.split('-')
    e_d = int(split_end_date[0])
    e_m = int(split_end_date[1])
    e_y = int(split_end_date[2])

    if s_y < 2019 or e_y < 2019:
        return False
    if s_y > 2022 or e_y > 2022:
        return False
    if (s_y == 2019 and s_m < 12) or (e_y == 2019 and e_y == 2022):
        return False

    return True
