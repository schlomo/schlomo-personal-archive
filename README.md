## Schlomo Personal Archive

{% assign doclist = site.pages | sort: 'url'  %}
    <ul>
       {% for doc in doclist %}
                <li><a href="{{ doc.url }}">{{ doc.url }}</a></li>
        {% endfor %}
    </ul>
