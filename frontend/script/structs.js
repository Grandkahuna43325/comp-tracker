export class Comp {
  constructor(id, dateRange, details, ageCategory, location, name) {
    this.id = id;
    this.dateRange = this.parseDateRange(dateRange);
    this.details = this.parseDetails(details);
    this.ageCategory = ageCategory;
    this.location = location;
    this.name = name;
  }

  parseDateRange(dateRange) {
    if (!dateRange) return { start: null, end: null };
    const [start, end] = dateRange
    .split(" - ")
    .map((date) => new Date(date + "T00:00:00Z"));
    let res = new Array();
    res.push(start);
    res.push(end);
    return res;
  }

  parseDetails(details) {
    if (!details) return {};
    const parsedDetails = {};

    for (const [event, result] of Object.entries(details)) {
      if (!result) continue;
      const [placement, participants, time] = result.split(" : ");
      parsedDetails[event] = {
        placement: placement || null,
        overall: participants === "null" ? null : participants,
        time: time || null,
      };
    }

    return parsedDetails;
  }

  addDetail(event, placement, participants, time) {
    this.details[event] = {
      placement: placement || null,
      participants: participants || null,
      time: time || null,
    };
  }

  getDetail(event) {
    return this.details[event];
  }

  static fromJSON(jsonData) {
    const records = new Map();

    for (const [id, data] of Object.entries(jsonData)) {
      records.set(
        id,
        new Comp(
          id,
          data.date_range,
          data.details,
          data.age_cathegory,
          data.location,
          data.name,
        ),
      );
    }

    return records;
  }

  static findById(records, id) {
    return records.get(id.toString());
  }

  static findByDateRange(records, startDate, endDate) {
    const start = new Date(startDate);
    const end = new Date(endDate);

    return Array.from(records.values()).filter((record) => {
      if (!record.dateRange.start || !record.dateRange.end) return false;

      return record.dateRange.start >= start && record.dateRange.end <= end;
    });
  }
}

