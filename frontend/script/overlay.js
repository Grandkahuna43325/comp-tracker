import { overlay } from "../index.js";

export let edit_mode = false;
export const overlay_input = document.createElement("input");
overlay_input.id = "overlay-input"

let table_mode = false;

let current_el = null;

let current_row = null;
let current_col = null;

const overlay_name_el = document.createElement("h2");
const overlay_date_el = document.createElement("p");
const overlay_location_el = document.createElement("p");
const overlay_age_cat_el = document.createElement("p");
const overlay_table_el = document.createElement("table");
const overlay_thead_el = document.createElement("thead");
const overlay_tbody_el = document.createElement("tbody");

export function overlay_accept_input() {
  if (document.activeElement != overlay_input) return void "";

  let textNode = document.createTextNode(overlay_input.value);
  current_el.replaceChild(textNode, overlay_input);
}

export function overlay_start_edit() {
  if (edit_mode) {
    return void "";
  }
  edit_mode = true;
  current_el = overlay.firstChild;
  current_el.id = "overlay-selected";
  console.log(current_el);
}

export function overlay_start_input() {
  if (!edit_mode) return void "";

  if (table_mode) {
    console.log("table_mode");
    console.log(current_el);
    return void "";
  }

  overlay_input.value = current_el.childNodes[1].data;
  current_el.replaceChild(overlay_input, current_el.childNodes[1]);
  overlay_input.focus();
}

export function overlay_exit_edit_mode() {
  if (current_el) current_el.id = "";
  current_el = null;

  if (table_mode) {
    current_row.id = "";
    current_col.id = "";
    current_row = null;
    current_col = null;
    table_mode = false;
    current_el = overlay.firstChild;
    current_el.id = "overlay-selected";
    return void "";
  }

  edit_mode = false;
}

export function overlay_exit_table_mode() {
  current_el = null;
  current_row.id = "";
  current_col.id = "";
  current_row = null;
  current_col = null;
  table_mode = false;
}

export function overlay_go_right() {
  if (!edit_mode || !table_mode || !current_row) {
    return void "";
  }
  if (current_col) current_col.id = "";

  current_col =
    current_col == null
      ? current_row.children.item(0)
      : current_col.nextElementSibling;
  while (current_col == null) {
    current_col =
      current_col == null
        ? current_row.children.item(0)
        : current_col.nextElementSibling;
  }

  if (current_col) current_col.id = "overlay-selected-td";
}

export function overlay_go_left() {
  if (!edit_mode || !table_mode || !current_row) {
    return void "";
  }
  if (current_col) current_col.id = "";

  current_col =
    current_col == null
      ? current_row.children.item(current_row.children.length - 1)
      : current_col.previousElementSibling;
  while (current_col == null) {
    current_col =
      current_col == null
        ? current_row.children.item(current_row.children.length - 1)
        : current_col.previousElementSibling;
  }

  if (current_col) current_col.id = "overlay-selected-td";
}

export function overlay_go_down() {
  if (edit_mode && table_mode) {
    if (current_row) {
      current_row.id = "";
      current_col.id = "";
    }

    current_row =
      current_row == null
        ? overlay_tbody_el.children.item(0)
        : current_row.nextElementSibling;
    while (current_row == null) {
      current_row =
        current_row == null
          ? overlay_tbody_el.children.item(0)
          : current_row.nextElementSibling;
    }

    if (current_row) {
      current_row.id = "overlay-selected";

      current_col = current_row.firstChild;
      current_col.id = "overlay-selected-td";
    }
    return void "";
  }

  if (edit_mode) {
    current_el.id = "";

    current_el =
      current_el == null
        ? overlay.children.item(0)
        : current_el.nextElementSibling;
    while (current_el == null) {
      current_el =
        current_el == null
          ? overlay.children.item(0)
          : current_el.nextElementSibling;
    }

    if (current_el.tagName == "TABLE") {
      table_mode = true;
    } else {
      current_el.id = "overlay-selected";
    }

    overlay.scrollTo(current_el);
    return void "";
  }

  overlay.scrollTop += 10;
}

export function overlay_go_up() {
  if (edit_mode && table_mode) {
    if (current_row) {
      current_row.id = "";
      current_col.id = "";
    }
    current_row =
      current_row == null
        ? overlay_tbody_el.children.item(overlay_tbody_el.children.length - 1)
        : current_row.previousElementSibling;
    while (current_row == null) {
      current_row =
        current_row == null
          ? overlay_tbody_el.children.item(overlay_tbody_el.children.length - 1)
          : current_row.previousElementSibling;
    }
    if (current_row) {
      current_row.id = "overlay-selected";

      current_col = current_row.firstChild;
      current_col.id = "overlay-selected-td";
    }
    return void "";
  }

  if (edit_mode) {
    current_el.id = "";

    current_el =
      current_el == null
        ? overlay.lastChild
        : current_el.previousElementSibling;
    while (current_el == null) {
      current_el =
        current_el == null
          ? overlay.lastChild
          : current_el.previousElementSibling;
    }

    if (current_el.tagName == "TABLE") {
      table_mode = true;
    } else {
      current_el.id = "overlay-selected";
    }

    overlay.scrollTo(current_el);
    return void "";
  }

  overlay.scrollTop -= 10;
}

export function show_selected() {
  overlay.innerHTML = "";
  overlay_tbody_el.innerHTML = "";
  let data = JSON.parse(
    comp_list.children.item(comp_list.dataset.current_el).dataset.all,
  );

  overlay.classList.remove("hidden");

  let parsed_date =
    data.dateRange[0].slice(0, 10) + " - " + data.dateRange[1].slice(0, 10);

  overlay_name_el.innerHTML = `<strong></strong>${data.name}`;
  overlay.appendChild(overlay_name_el);

  overlay_date_el.innerHTML = `<strong>Data:</strong> ${parsed_date}`;
  overlay.appendChild(overlay_date_el);

  overlay_location_el.innerHTML = `<strong>Miejsce:</strong> ${data.location}`;
  overlay.appendChild(overlay_location_el);

  overlay_age_cat_el.innerHTML = `<strong>Kategoria wiekowa:</strong> ${data.ageCathegory ? data.ageCathegory : "-"}`;
  overlay.appendChild(overlay_age_cat_el);

  overlay_table_el.className = "result-table";

  overlay_thead_el.innerHTML = `<tr><th>Dystans</th><th>Kategoria</th><th>Open</th><th>Czas</th></tr>`;
  overlay_table_el.appendChild(overlay_thead_el);

  for (let [key, val] of Object.entries(data.details)) {
    const tr = document.createElement("tr");

    const td1 = document.createElement("td");
    td1.textContent = key;
    tr.appendChild(td1);

    const td2 = document.createElement("td");
    td2.textContent = val.placement || "-";
    tr.appendChild(td2);

    const td3 = document.createElement("td");
    td3.textContent = val.overall || "-";
    tr.appendChild(td3);

    const td4 = document.createElement("td");
    td4.textContent = val.time || "-";
    tr.appendChild(td4);

    overlay_tbody_el.appendChild(tr);
  }

  overlay_table_el.appendChild(overlay_tbody_el);
  overlay.appendChild(overlay_table_el);
}
