{% for item in items -%} 
<a href="{{ item.url }}">{% if item.thumbUrl != "" %}<img src="{{ item.thumbUrl }}" alt=""/>{% else %}{{ item.fileName }}{% endif %}</a>{% if item.rowEnd %}<br/>&nbsp;<br/>{% else if not loop.is_last%}&nbsp;&nbsp;{% endif %}
{% endfor -%}