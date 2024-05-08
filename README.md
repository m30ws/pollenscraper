# pollenscraper

Extracts today's pollen levels for a select few of Croatian cities obtained from a HTML page in specific format at ex. https://www.plivazdravlje.hr/alergije/prognoza/1/Zagreb.html

Works by simply searching for defined keyword (or keywords) iteratively and processing each one, which means manual analysis and selection of said keywords is required, as well as potential additional processing, before any data can be parsed.
Also able to export data as JSON to a file or to STDOUT.

It does not feature downloading of the actual data but expects it to be provided either from a file or from STDIN. It's not intended to be used as a library of any sort but rather a relatively simple tool easily changed and extendable for usage with other specific pages as needed.

## Usage

### Compile
```cmd
gcc extract.c -o extract
```

### Run
- input data from file:
	```cmd
	./extract page.html
	```

- input from STDIN:
	```cmd
	./extract 
	./extract < page.html
	```

Enabling/disabling of automatic JSON export is controlled through `EXPORT_JSON` #define at the top. Set `JSON_FILE` to "" to output JSON data to STDOUT.
