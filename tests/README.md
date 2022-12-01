### Unit tests

| Test    | Script        | Requires  | Output
| :---:   | :---:         | :---:     | :---:
| Read    | read-test.sh  |           | output/read_output.json
| Write   | write-test.sh |           | output/write_output.json
| Execute | exec-test.sh  | python    | output/exec_output.json

To run each test:

1. Start the thoth daemon
 `sudo systemctl start thothd`
 
2. Run the test, example:
 `./write-test.sh`
 
3. Stop the thoth daemon
 `sudo systemctl stop thothd`
 
4. Compare the provenance log created in the `/tmp/` directory to the expected outputs in `output`. Note: some values like the pathname 
   and timestamp will be different, but logs should otherwise be similar.
