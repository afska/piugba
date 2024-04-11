const fs = require("fs");
const _ = require("./importer/node_modules/lodash");

const FILE_PATH = "../src/data/content/_compiled_sprites/palette_test.c";

const file = fs.readFileSync(FILE_PATH).toString();
const content = file.substring(file.indexOf("0x"), file.indexOf("};"));
const palette = eval(`[${content}]`);

console.log(
  JSON.stringify(
    palette.map(
      (it) => {
        const r = it & 0b11111;
        const g = (it >> 5) & 0b11111;
        const b = (it >> 10) & 0b11111;

        return [
          "#" +
            Math.round((r * 0xff) / 0b11111)
              .toString(16)
              .padStart(2, "0") +
            Math.round((g * 0xff) / 0b11111)
              .toString(16)
              .padStart(2, "0") +
            Math.round((b * 0xff) / 0b11111)
              .toString(16)
              .padStart(2, "0"),
          it,
        ];
      },
      null,
      2
    )
  )
);

/*
<html>
<body>
<div id="palette">

<script>

const palette = document.querySelector("#palette");

const colors =
[["#ff00ff",31775],["#2100f7",30724],["#6b00de",27661],["#a500d6",26644],["#0031f7",30912],["#ff0800",63],["#000000",0],["#4a5242",8521],["#005aef",30048],["#6b6b6b",13741],["#f75200",350],["#008cf7",31264],["#009cce",26208],["#00a5ff",32384],["#00adce",26272],["#849494",19024],["#94948c",18002],["#94948c",18002],["#f78400",542],["#9ca59c",20115],["#00d6a5",21312],["#4ace00",809],["#00d6d6",27456],["#f79c08",1662],["#adadad",22197],["#10dec5",25442],["#00f700",960],["#00f742",9152],["#f7b500",734],["#00f794",19392],["#94d600",850],["#c5c5c5",25368],["#b5e600",918],["#efd600",861],["#d6de00",890],["#d6d6d6",27482],["#ffe600",927],["#eff700",989],["#e6efe6",29628]]

palette.innerHTML = colors.map(([it, c],i) => `<div style="width: 100px; height: 100px; background-color: ${it}">${i}, 0x${c.toString(16)}</div>`).join("");

</script>
</body>
</html>
*/
