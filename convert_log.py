import json
import sys

def convert_timestamp_to_milliseconds(log_start, timestamp):
    timestamp = int(timestamp) / 1e6
    log_start = int(log_start) / 1e6
    return timestamp - log_start

# Very crude, works for now
def parse_log_line(line):
    parts = line.strip().split(',')
    if len(parts) >= 4 and parts[0].endswith("EVENT"):
        event_name = parts[1]
        timestamp = parts[2]
        extra_data = parts[3]
        return event_name, timestamp, extra_data
    return None

def convert(log_file):
    event_pairs = {
        "lfx_beginframe": "lfx_endframe",
        "marker_SIMULATION_START": "marker_SIMULATION_END",
        "marker_RENDERSUBMIT_START": "marker_RENDERSUBMIT_END",
        "marker_PRESENT_START": "marker_PRESENT_END"
    }
    in_progress_events = {}
    events = []

    with open(log_file, 'r') as file:
        log_lines = file.readlines()
        if not log_lines:
            return None

        # Find earliest timestamp
        first_event = None
        for line in log_lines:
            first_event = parse_log_line(line)
            if first_event:
                break

        if first_event is None:
            raise ValueError("No valid log lines found in the log file.")

        log_start_time = int(first_event[1])

        for line in log_lines:
            parsed = parse_log_line(line)
            if parsed:
                event_name, timestamp, extra_data = parsed
                timestamp_ns = int(timestamp)
                time_in_us = (timestamp_ns - log_start_time) // 1000

                if event_name in event_pairs:
                    if extra_data not in in_progress_events:
                        in_progress_events[extra_data] = []
                    in_progress_events[extra_data].append((event_name, time_in_us))
                elif event_name in event_pairs.values():
                    start_event_name = next((k for k, v in event_pairs.items() if v == event_name), None)
                    if start_event_name and extra_data in in_progress_events:
                        matching_event = next((evt for evt in in_progress_events[extra_data] if evt[0] == start_event_name), None)
                        if matching_event:
                            in_progress_events[extra_data].remove(matching_event)
                            begin_time = matching_event[1]
                            events.append({
                                "name": start_event_name.replace("_START", "").replace("begin", ""),
                                "cat": "LatencyFlex" if "lfx" in start_event_name else "Marker",
                                "ph": "X",
                                "ts": begin_time,
                                "dur": time_in_us - begin_time,
                                "args": {
                                    "frame_id": extra_data,
                                }
                            })
                        if not in_progress_events[extra_data]:
                            del in_progress_events[extra_data]
                elif event_name == "lfx_sleep":
                    duration_us = int(extra_data) // 1000
                    events.append({
                        "name": event_name,
                        "cat": "LatencyFlex",
                        "ph": "X",
                        "ts": time_in_us,
                        "dur": duration_us,
                    })
                else:
                    events.append({
                        "name": event_name,
                        "cat": "General",
                        "ph": "i",
                        "ts": time_in_us,
                        "args": {
                            "frame_id": extra_data
                        }
                    })
    return events

def main():
    if len(sys.argv) > 1:
        log_file = sys.argv[1]
        output_file = 'trace_events.json'

        trace_events = convert(log_file)
        if trace_events:
            with open(output_file, 'w') as file:
                json.dump(trace_events, file, indent=4)
            print("Trace event data saved to", output_file)
        else:
            print("No data to convert.")
    else:
        print("No path provided")

if __name__ == "__main__":
    main()
