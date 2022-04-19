run:
	(cat ../dataset/event1.txt | python mapper.py | sort | python combiner.py \
	& cat ../dataset/event2.txt | python mapper.py | sort | python combiner.py \
	& cat ../dataset/event3.txt | python mapper.py | sort | python combiner.py \
	& cat ../dataset/event4.txt | python mapper.py | sort | python combiner.py \
	& cat ../dataset/event5.txt | python mapper.py | sort | python combiner.py \
	& cat ../dataset/event6.txt | python mapper.py | sort | python combiner.py \
	& cat ../dataset/event7.txt | python mapper.py | sort | python combiner.py \
	& cat ../dataset/event8.txt | python mapper.py | sort | python combiner.py \
	& cat ../dataset/event9.txt | python mapper.py | sort | python combiner.py \
	& cat ../dataset/event10.txt | python mapper.py | sort | python combiner.py) \
	| sort | python reducer.py > result_event.txt