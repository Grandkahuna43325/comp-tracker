import { input, comp_list } from "../index.js";
import { fuzzyMatch as fuzzyFind } from "../lib/FuzzyMatch/js-fuzzyMatch.js";
import { comps } from "./backend.js";


export function find_match() {
  const phrase = input.value.toLowerCase();
  const list_len = comp_list.childElementCount;

  let i = 0;
  for (; i < list_len; i++) {
    var e = comp_list.children.item(i);
    e.dataset.fs = e.textContent;
  }

  if (phrase != "") {
    fuzzyFind.prep(phrase);

    for (i = 0; i < list_len; i++) {
      var e = comp_list.children.item(i);
      if (fuzzyFind.match(e.dataset.all)) {
        e.style.display = "";
        // e.innerHTML = fuzzyFind.hiblock(e.dataset.fs);
        e.innerHTML = fuzzyFind.hi(e.dataset.fs);
      } else {
        e.style.display = "none";
      }
    }
  } else {
    for (i = 0; i < list_len; i++) {
      var e = comp_list.children.item(i);
      e.style.display = "";
    }
  }
}

export function focus_next_el() {
  const list_len = comp_list.childElementCount;
  comp_list.children.item(comp_list.dataset.current_el).classList.remove("highlited");
  comp_list.dataset.current_el = (comp_list.dataset.current_el + 1) % list_len;
  while (comp_list.children.item(comp_list.dataset.current_el).style.display != "") {
    comp_list.dataset.current_el = (comp_list.dataset.current_el + 1) % list_len;
  }
  comp_list.children.item(comp_list.dataset.current_el).classList.add("highlited");
}

export function focus_prev_el() {
  const list_len = comp_list.childElementCount;
  comp_list.children.item(comp_list.dataset.current_el).classList.remove("highlited");
  comp_list.dataset.current_el = (comp_list.dataset.current_el - 1 + list_len) % list_len;
  while (comp_list.children.item(comp_list.dataset.current_el).style.display != "") {
    comp_list.dataset.current_el = (comp_list.dataset.current_el - 1 + list_len) % list_len;
  }
  comp_list.children.item(comp_list.dataset.current_el).classList.add("highlited");
}

export function display_comp_search_list() {
  comp_list.innerHTML = "";
  comp_list.id = "comp_list";

  comps.forEach((el, idx) => {
    let content = document.createElement("li");
    if (idx == 1) {
      content.classList.add("highlited");
    }
    content.innerText =
      el.location + " " + el.dateRange[0].toISOString().split("T")[0];
    content.dataset.all = JSON.stringify(el);

    comp_list.appendChild(content);
  });

  comp_list.dataset.current_el = 0;
}
