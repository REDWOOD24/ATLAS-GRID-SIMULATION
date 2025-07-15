from datetime import datetime

def find_time_difference(time_str1, time_str2):
  """
  Calculates the difference between two time strings.

  Args:
    time_str1: The first time string in 'YYYY-MM-DD HH:MM:SS.ffffff' format.
    time_str2: The second time string in 'YYYY-MM-DD HH:MM:SS.ffffff' format.

  Returns:
    A timedelta object representing the difference between the two times,
    or None if the strings cannot be parsed.
  """
  # Define the format of the time strings.
  # %Y - Year with century
  # %m - Month as a zero-padded decimal number.
  # %d - Day of the month as a zero-padded decimal number.
  # %H - Hour (24-hour clock) as a zero-padded decimal number.
  # %M - Minute as a zero-padded decimal number.
  # %S - Second as a zero-padded decimal number.
  # %f - Microsecond as a decimal number, zero-padded on the left.
  time_format = "%Y-%m-%d %H:%M:%S.%f"

  try:
    # Convert the time strings into datetime objects
    t1 = datetime.strptime(time_str1, time_format)
    t2 = datetime.strptime(time_str2, time_format)

    # Calculate the absolute difference between the two datetime objects
    time_diff = abs(t2 - t1)
    return time_diff
  except ValueError as e:
    print(f"Error: Could not parse time strings. Please check the format. Details: {e}")
    return None

# --- Main execution part of the script ---
if __name__ == "__main__":
  # The two time strings provided by the user
  start_time = "2025-06-23 15:16:47.566"
  end_time = "2025-06-23 15:17:37.935"

  print(f"Calculating difference between:")
  print(f"Start time: {start_time}")
  print(f"End time:   {end_time}")
  print("-" * 30)

  # Call the function to get the time difference
  difference = find_time_difference(start_time, end_time)

  if difference:
    # The result is a timedelta object, which is easy to read
    print(f"Time Difference: {difference}")

    # You can also get the total difference in seconds
    total_seconds = difference.total_seconds()
    print(f"Total Difference in Seconds: {total_seconds}")