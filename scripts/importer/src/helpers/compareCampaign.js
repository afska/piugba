const BASE = `INSERT_BASE_TABLE`;
const UPDATE = `INSERT_UPDATE_TABLE`;

function parse(data) {
  const lines = data.trim().split("\n");
  const parsedData = [];

  lines.slice(4, -1).forEach((line) => {
    const columns = line.split("│").map((col) => col.trim());

    if (columns.length >= 7) {
      const title = columns[2];
      const normal = parseInt(columns[5].split("(")[0].trim(), 10);
      const hard = parseInt(columns[6].split("(")[0].trim(), 10);
      const crazy = parseInt(columns[7].split("(")[0].trim(), 10);

      parsedData.push({ title, normal, hard, crazy });
    }
  });

  return parsedData;
}

const baseData = parse(BASE);
const updateData = parse(UPDATE);

updateData.forEach((song) => {
  const dataInBase = baseData.find((it) => it.title === song.title);
  if (dataInBase == null) throw new Error("song_not_found");

  if (
    song.normal !== dataInBase.normal ||
    song.hard !== dataInBase.hard ||
    song.crazy !== dataInBase.crazy
  )
    console.log(
      song.title,
      "expected",
      dataInBase.normal,
      dataInBase.hard,
      dataInBase.crazy,
      "but got",
      song.normal,
      song.hard,
      song.crazy
    );
});
